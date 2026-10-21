// tableGeneratorMain_mpi_timeout.cpp
#include <sstream>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <cmath>
#include <iomanip>
#include <vector>
#include <string>
#include <ctime>
#include <algorithm>
#include <cerrno>
#include <set>

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>

#include "Amplitudes.h"
#include "TROOT.h"
#include "TH1D.h"
#include "TFile.h"
#include "Nucleus.h"
#include "Constants.h"
#include "Table.h"
#include "TableGeneratorSettings.h"
#include "Enumerations.h"
#include "DglapEvolution.h"
#include "Version.h"
#include "TRandom3.h"
#include "InclusiveDiffractiveCrossSections.h"

#include "mpi.h"
#include <fstream>
#define PR(x) cout << #x << " = " << (x) << endl;

using namespace std;

///////////////////////////////////////////////////////////////////////////////
// Manual Switches for Table Generation
///////////////////////////////////////////////////////////////////////////////
// Select which Quark Flavors (q0, q1, q2, q3) to compute
const bool calc_q[4] = { true, true, true, true };

// Select which Cross-Section Channels to compute
const bool calc_TQQ  = true;  // Transverse QQ
const bool calc_LQQ  = true;  // Longitudinal QQ
const bool calc_TQQG = true;  // Transverse QQG (set calc_TQQ and calc_LQQ to false if you ONLY want QQG)

///////////////////////////////////////////////////////////////////////////////
// Checkpointing configuration
///////////////////////////////////////////////////////////////////////////////
// How often (in percent of total bins in this run) to persist progress.
// Kept in the 10-20% range you asked for; lower = safer but more I/O.
const double CHECKPOINT_PERCENT_STEP = 1.0;

// One record per completed bin: the bin index plus the 12 doubles that were
// being sent over MPI anyway (TQQ/LQQ/TQQG per quark flavor). Storing the
// actual values (not just "this bin is done") means a resume never has to
// recompute anything - we just refill the in-memory Table objects from disk.
struct BinRecord {
    int bin;
    double vals[12];
};

///////////////////////////////////////////////////////////////////////////////
// Graceful shutdown handling
///////////////////////////////////////////////////////////////////////////////
// Many schedulers (SLURM, PBS, k8s, etc.) send SIGTERM some time before the
// hard SIGKILL. Catching it lets the master flush whatever has been computed
// so far instead of losing an entire in-progress percent bucket.
static volatile sig_atomic_t g_shutdownRequested = 0;
void handleShutdownSignal(int /*sig*/) {
    g_shutdownRequested = 1;
}

///////////////////////////////////////////////////////////////////////////////
// Helper: reliable write / read
///////////////////////////////////////////////////////////////////////////////
ssize_t write_all(int fd, const void *buf, size_t count) {
    const char *p = (const char*)buf;
    size_t left = count;
    while (left > 0) {
        ssize_t written = write(fd, p, left);
        if (written < 0) {
            if (errno == EINTR) continue;
            return -1;
        }
        left -= static_cast<size_t>(written);
        p += written;
    }
    return static_cast<ssize_t>(count);
}

ssize_t read_all(int fd, void *buf, size_t count) {
    char *p = (char*)buf;
    size_t left = count;
    while (left > 0) {
        ssize_t r = read(fd, p, left);
        if (r < 0) {
            if (errno == EINTR) continue;
            return -1;
        }
        if (r == 0) break; // EOF
        left -= static_cast<size_t>(r);
        p += r;
    }
    return static_cast<ssize_t>(count - left);
}

///////////////////////////////////////////////////////////////////////////////
// computeBinWithTimeout
///////////////////////////////////////////////////////////////////////////////
int computeBinWithTimeout(
    int bin,
    InclusiveDiffractiveCrossSections* idcs_parent,
    Table tableTQQG[4],
    double out[12],
    int timeout_ms
) {
    int pipefd[2];
    if (pipe(pipefd) < 0) {
        perror("pipe");
        return -1;
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        close(pipefd[0]); close(pipefd[1]);
        return -1;
    }

    if (pid == 0) {
        // CHILD PROCESS
        close(pipefd[0]);

        double Q2 = 0, W2 = 0, z = 0, beta = 0;
        tableTQQG[0].binCenter(bin, Q2, W2, beta, z);

        if (calc_TQQG) {
            idcs_parent->dsigdbetadQ2dW2dz_qqg(beta, Q2, W2, z);
        }
        if (calc_TQQ) {
            idcs_parent->dsigdbetadQ2dW2dz_total(beta, Q2, W2, z, transverse);
        }
        if (calc_LQQ) {
            idcs_parent->dsigdbetadQ2dW2dz_total(beta, Q2, W2, z, longitudinal);
        }

        double temp[12] = {0.0};
        int idx = 0;
        for (int iq = 0; iq < 4; ++iq) {
            if (calc_q[iq]) {
                temp[idx++] = calc_TQQ  ? idcs_parent->dsigdbetadQ2dWdz_T_total()[iq] : 0.0;
                temp[idx++] = calc_LQQ  ? idcs_parent->dsigdbetadQ2dWdz_L_total()[iq] : 0.0;
                temp[idx++] = calc_TQQG ? idcs_parent->dsigdbetadQ2dWdz_T_qqg()[iq]    : 0.0;
            } else {
                idx += 3;
            }
        }

        if (write_all(pipefd[1], temp, sizeof(temp)) < 0) {
            _exit(2);
        }
        close(pipefd[1]);

        _exit(0);
    }

    // PARENT PROCESS
    close(pipefd[1]);

    int status = 0;
    bool finished = false;

    // Poll with growing intervals: start fast so quick bins are caught
    // almost immediately, cap the growth so we don't busy-poll on slow ones.
    long waited_us = 0;
    long step_us = 200;              // start at 0.2 ms
    const long step_cap_us = 20000;  // cap at 20 ms
    const long timeout_us = static_cast<long>(timeout_ms) * 1000;

    while (waited_us < timeout_us) {
        usleep(step_us);
        waited_us += step_us;

        pid_t r = waitpid(pid, &status, WNOHANG);
        if (r == pid) {
            finished = true;
            break;
        }

        if (step_us < step_cap_us) step_us *= 2;
    }

    if (finished) {
        ssize_t nr = read_all(pipefd[0], out, sizeof(double) * 12);
        if (nr < 0) {
            perror("read");
            close(pipefd[0]);
            for (int i = 0; i < 12; ++i) out[i] = 1e-50;
            return 0;
        }
        close(pipefd[0]);
        return 1;
    }

    ofstream logfile("timeout_bins.txt", std::ios::app);
    logfile << bin << "\n";
    logfile.close();

    kill(pid, SIGKILL);
    waitpid(pid, &status, 0);
    close(pipefd[0]);

    for (int i = 0; i < 12; ++i) out[i] = 1e-50;  // unified with the read-error sentinel (was 1, see note below)
    return 0;
}

///////////////////////////////////////////////////////////////////////////////
// Main program
///////////////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[]) {

    MPI_Init(&argc, &argv);
    MPI_Status status;

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    const int MASTER = 0;
    const int SIZE_TAG   = 0;
    const int BIN_TAG    = 1;
    const int RESULT_TAG = 2;
    const int STOP_TAG   = 3;
    const int chunksize  = 1;

    const int TIMEOUT_MS = 1e6;

    time_t theStartTime = time(0);
    string ctstr(ctime(&theStartTime));
    ctstr.erase(ctstr.size()-1, 1);

    TH1::AddDirectory(false);
    TableGeneratorSettings* settings = TableGeneratorSettings::instance();

    char* runcard = argv[1];
    if (rank == MASTER) cout << "Reading settings from runcard." << endl;
    settings->readSettingsFromFile(runcard);
    settings->consolidateSettings();

    EventGeneratorSettings* egsettings = EventGeneratorSettings::instance();
    egsettings->setDipoleModelParameterSet(settings->dipoleModelParameterSet());
    egsettings->setDipoleModelType(settings->dipoleModelType());
    egsettings->setA(settings->A());
    egsettings->setVerboseLevel(settings->verboseLevel());
    egsettings->setApplyPhotonFlux(false);

    int nBinbeta = settings->betabins();
    int nBinQ2   = settings->Q2bins();
    int nBinW2   = settings->W2bins();
    int nBinz    = settings->zbins();
    double betamin = settings->betamin();
    double betamax = settings->betamax();
    double Q2min  = settings->Q2min();
    double Q2max  = settings->Q2max();
    double Wmin   = settings->Wmin();
    double Wmax   = settings->Wmax();
    double W2min  = Wmin*Wmin;
    double W2max  = Wmax*Wmax;
    double zmin   = settings->zmin();
    double zmax   = settings->zmax();
    unsigned int massA = settings->A();
    DipoleModelType model = settings->dipoleModelType();
    DipoleModelParameterSet pset = settings->dipoleModelParameterSet();
    int startingBin = settings->startBin();
    int endingBin   = settings->endBin();
    unsigned char priority = settings->priority();
    bool hotspots = settings->hasSubstructure();
    double fractionOfBinsToFill = settings->fractionOfBinsToFill();

    settings->setStartBin(0);
    settings->setEndBin(nBinbeta*nBinQ2*nBinW2*nBinz);

    if (rank == MASTER) settings->list();

    int maxbins = nBinbeta*nBinQ2*nBinW2*nBinz;
    if (endingBin >= maxbins) {
        if (rank == MASTER) {
            cout << "Warning, given end bin (" << endingBin << ") exceeds the size of the table." << endl;
            cout << "         set to maximum value (" << maxbins - 1 << ") now." << endl;
        }
        endingBin = maxbins - 1;
    }

    Table tableTQQ[4];
    Table tableLQQ[4];
    Table tableTQQG[4];

    bool logbeta = false, logQ2 = true, logW2 = true, logz = false, logC = true;

    string rootfile = settings->rootfile();
    rootfile += "_" + settings->dipoleModelName();
    rootfile += "_" + settings->dipoleModelParameterSetName();

    ostringstream filenameTQQ[4], filenameLQQ[4], filenameTQQG[4];

    // Selective Table Creation
    for (int i = 0; i < 4; i++) {
        if (!calc_q[i]) continue; // Skip unselected quark flavors

        if (calc_TQQ) {
            filenameTQQ[i].str("");
            filenameTQQ[i] << rootfile << "_q" << i << "_bin" << startingBin << "-" << endingBin << "_TQQ.root";
            (void) tableTQQ[i].create(nBinbeta, betamin, betamax,
                                      nBinQ2, Q2min, Q2max,
                                      nBinW2, W2min, W2max,
                                      nBinz,  zmin,  zmax,
                                      logbeta, logQ2, logW2, logz, logC,
                                      mean_A, transverse, QQ,
                                      massA, model, pset, i,
                                      filenameTQQ[i].str().c_str(),
                                      priority, hotspots);
        }

        if (calc_LQQ) {
            filenameLQQ[i].str("");
            filenameLQQ[i] << rootfile << "_q" << i << "_bin" << startingBin << "-" << endingBin << "_LQQ.root";
            (void) tableLQQ[i].create(nBinbeta, betamin, betamax,
                                      nBinQ2, Q2min, Q2max,
                                      nBinW2, W2min, W2max,
                                      nBinz,  zmin,  zmax,
                                      logbeta, logQ2, logW2, logz, logC,
                                      mean_A, longitudinal, QQ,
                                      massA, model, pset, i,
                                      filenameLQQ[i].str().c_str(),
                                      priority, hotspots);
        }

        if (calc_TQQG) {
            filenameTQQG[i].str("");
            filenameTQQG[i] << rootfile << "_q" << i << "_bin" << startingBin << "-" << endingBin << "_TQQG.root";
            (void) tableTQQG[i].create(nBinbeta, betamin, betamax,
                                       nBinQ2, Q2min, Q2max,
                                       nBinW2, W2min, W2max,
                                       nBinz,  zmin,  zmax,
                                       logbeta, logQ2, logW2, logz, logC,
                                       mean_A, transverse, QQG,
                                       massA, model, pset, i,
                                       filenameTQQG[i].str().c_str(),
                                       priority, hotspots);
        }
    }

    if (rank == MASTER) {
        cout << "\nActive configuration selected:" << endl;
        for (int i = 0; i < 4; i++) {
            if (calc_q[i]) {
                cout << "  - Flavor q" << i << ":"
                     << (calc_TQQ  ? " [TQQ]"  : "")
                     << (calc_LQQ  ? " [LQQ]"  : "")
                     << (calc_TQQG ? " [TQQG]" : "") << endl;
            }
        }
        cout << "Tables have " << maxbins << " bins each.\n" << endl;
        cout << "Automatic backup of tables is off.\n" << endl;
    }

    TRandom3 *random = TableGeneratorSettings::randomGenerator();
    random->SetSeed();

    DglapEvolution &dglap = DglapEvolution::instance();
    dglap.generateLookupTable(1000, 1000);
    dglap.useLookupTable(true);

    if (rank == MASTER) {
        cout << endl;
        cout << "Tables will be generated for:" << endl;
        cout << "\tNucleus mass A=" << massA << endl;
        cout << "\tBins: " << startingBin << "-" << endingBin << endl;
        cout << "\tbeta range: [" << betamin << ", " << betamax << "], " << nBinbeta << " bins." << endl;
        cout << "\tQ2 range: [" << Q2min << ", " << Q2max << "], " << nBinQ2 << " bins." << endl;
        cout << "\tW2 range: [" << W2min << ", " << W2max << "], " << nBinW2 << " bins." << endl;
        cout << "\tz range: [" << zmin << ", " << zmax << "], " << nBinz << " bins." << endl;
        cout << "\tDipole model: " << settings->dipoleModelName() << endl;
        cout << "\tDipole model parameter set: " << settings->dipoleModelParameterSetName() << endl;
        cout << "\tTable set mode: " << settings->tableSetTypeName() << endl;
        cout << endl;

        // -------------------------------------------------------------
        // Checkpoint setup / resume
        // -------------------------------------------------------------
        // Install a handler so SIGTERM/SIGINT (typical scheduler pre-kill
        // signals) trigger an immediate save instead of just dying.
        signal(SIGTERM, handleShutdownSignal);
        signal(SIGINT,  handleShutdownSignal);

        string checkpointPath = rootfile + "_bin" + std::to_string(startingBin) + "-"
                                 + std::to_string(endingBin) + "_checkpoint.dat";

        set<int> completedBins;
        std::vector<BinRecord> pendingCheckpointRecords;

        // If a checkpoint from a previous (interrupted) run exists, load it:
        // refill the in-memory tables with the already-computed values so
        // we never recompute a bin we already have.
        {
            ifstream chkIn(checkpointPath, ios::binary);
            if (chkIn.good()) {
                BinRecord rec;
                long count = 0;
                while (chkIn.read(reinterpret_cast<char*>(&rec), sizeof(BinRecord))) {
                    int idx = 0;
                    for (int iq = 0; iq < 4; ++iq) {
                        double valTQQ  = rec.vals[idx++];
                        double valLQQ  = rec.vals[idx++];
                        double valTQQG = rec.vals[idx++];
                        if (calc_q[iq]) {
                            if (calc_TQQ)  tableTQQ[iq].fill(rec.bin, valTQQ, 0);
                            if (calc_LQQ)  tableLQQ[iq].fill(rec.bin, valLQQ, 0);
                            if (calc_TQQG) tableTQQG[iq].fill(rec.bin, valTQQG, 0);
                        }
                    }
                    completedBins.insert(rec.bin);
                    ++count;
                }
                chkIn.close();
                if (count > 0) {
                    double pct = 100.0 * count / (endingBin - startingBin + 1);
                    cout << "Found existing checkpoint '" << checkpointPath << "': "
                         << count << " bins already completed (" << fixed << setprecision(1)
                         << pct << "%). Resuming from there." << endl;
                }
            } else {
                cout << "No existing checkpoint found, starting fresh." << endl;
            }
        }

        // Open the checkpoint file for appending new results as they come in.
        // We use a raw POSIX fd (not ofstream) so we can fsync() it: a plain
        // flush() only pushes data from our process into the OS page cache,
        // which is still lost on a hard power failure. fsync() forces the OS
        // to actually commit it to disk, which is what protects against
        // "the computer shuts down" (as opposed to just "the process dies").
        int checkpointFd = open(checkpointPath.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0644);
        if (checkpointFd < 0) {
            cout << "WARNING: could not open checkpoint file '" << checkpointPath
                 << "' for writing. Progress will NOT be saved incrementally!" << endl;
        }

        double totalBinsThisRun = static_cast<double>(endingBin - startingBin + 1);
        double nextCheckpointPercent = CHECKPOINT_PERCENT_STEP;
        {
            double resumedPercent = 100.0 * completedBins.size() / totalBinsThisRun;
            while (nextCheckpointPercent <= resumedPercent) {
                nextCheckpointPercent += CHECKPOINT_PERCENT_STEP;
            }
        }

        // auto writeTableSnapshots = [&]() {
        //     for (int i = 0; i < 4; i++) {
        //         if (!calc_q[i]) continue;
        //         if (calc_TQQ)  tableTQQ[i].write();
        //         if (calc_LQQ)  tableLQQ[i].write();
        //         if (calc_TQQG) tableTQQG[i].write();
        //     }
        //     if (checkpointFd >= 0) fsync(checkpointFd);
        // };
        auto writeTableSnapshots = [&]() {
            for (int i = 0; i < 4; i++) {
                if (!calc_q[i]) continue;
            
                auto overwriteTable = [](auto& table) {
                    // Remove existing target file on disk if present
                    if (!table.filename().empty()) {
                        std::remove(table.filename().c_str());
                    }
                    // If filename() isn't available, pass the explicit path:
                    // table.write("your_table_name.root");
                    table.write();
                };
            
                if (calc_TQQ)  overwriteTable(tableTQQ[i]);
                if (calc_LQQ)  overwriteTable(tableLQQ[i]);
                if (calc_TQQG) overwriteTable(tableTQQG[i]);
            }
            if (checkpointFd >= 0) fsync(checkpointFd);
        };

        auto maybeCheckpoint = [&](bool force) {
            double percentDone = 100.0 * completedBins.size() / totalBinsThisRun;
            if (force || percentDone + 1e-9 >= nextCheckpointPercent) {
                cout << "Checkpoint: " << fixed << setprecision(1) << percentDone
                     << "% complete. Saving table snapshot + checkpoint file..." << endl;
                writeTableSnapshots();
                while (nextCheckpointPercent <= percentDone) {
                    nextCheckpointPercent += CHECKPOINT_PERCENT_STEP;
                }
            }
        };

        cout << "Start filling tables:" << endl;
        time_t tableFillStart = time(0);

        int nextBin = startingBin;
        int activeWorkers = 0;

        // Returns the next bin index that has NOT already been completed
        // (via a resumed checkpoint), or -1 if none remain.
        auto nextPendingBin = [&]() -> int {
            while (nextBin <= endingBin && completedBins.count(nextBin)) ++nextBin;
            if (nextBin > endingBin) return -1;
            return nextBin++;
        };

        for (int worker = 1; worker < size; ++worker) {
            int b = nextPendingBin();
            if (b < 0) break;
            vector<int> binIndices(1, b);
            int actualChunkSize = 1;
            MPI_Send(&actualChunkSize, 1, MPI_INT, worker, SIZE_TAG, MPI_COMM_WORLD);
            MPI_Send(binIndices.data(), actualChunkSize, MPI_INT, worker, BIN_TAG, MPI_COMM_WORLD);
            ++activeWorkers;
        }

        bool shutdownHandled = false;

        while (activeWorkers > 0) {
            int worker;
            int actualChunkSize;

            MPI_Status recvStatus;
            MPI_Recv(&actualChunkSize, 1, MPI_INT, MPI_ANY_SOURCE, SIZE_TAG, MPI_COMM_WORLD, &recvStatus);
            worker = recvStatus.MPI_SOURCE;

            vector<int> binIndices(actualChunkSize);
            MPI_Recv(binIndices.data(), actualChunkSize, MPI_INT, worker, BIN_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            int valuerec = 12 * actualChunkSize;
            vector<double> resultData(valuerec);
            MPI_Recv(resultData.data(), valuerec, MPI_DOUBLE, worker, RESULT_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            // Fill active tables. Checkpoint records are buffered in memory and
            // only flushed to disk (single write_all + single fsync) once we've
            // accumulated ~1% of total bins worth of records. Doing a write+fsync
            // per bin was dominating runtime; batching amortizes that cost while
            // still bounding data loss on a crash to roughly one percent-bucket.
            for (int i = 0; i < actualChunkSize; ++i) {
                int bin = binIndices[i];
                int idx = i * 12;

                if (checkpointFd >= 0) {
                    BinRecord rec;
                    rec.bin = bin;
                    for (int k = 0; k < 12; k++) rec.vals[k] = resultData[idx + k];
                    pendingCheckpointRecords.push_back(rec);
                }
                completedBins.insert(bin);

                for (int iq = 0; iq < 4; ++iq) {
                    double valTQQ  = resultData[idx++];
                    double valLQQ  = resultData[idx++];
                    double valTQQG = resultData[idx++];

                    if (calc_q[iq]) {
                        if (calc_TQQ) {
                            double val = (!isnan(valTQQ) && !isinf(valTQQ)) ? valTQQ : 1e-50;
                            tableTQQ[iq].fill(bin, val, 0);
                        }
                        if (calc_LQQ) {
                            double val = (!isnan(valLQQ) && !isinf(valLQQ)) ? valLQQ : 1e-50;
                            tableLQQ[iq].fill(bin, val, 0);
                        }
                        if (calc_TQQG) {
                            double val = (!isnan(valTQQG) && !isinf(valTQQG)) ? valTQQG : 1e-50;
                            tableTQQG[iq].fill(bin, val, 0);
                        }
                    }
                }
            }

            // Flush the buffered checkpoint records once we've reached ~1% of
            // totalBins since the last flush. One write_all() covering the whole
            // batch, then one fsync() for the batch.
            int onePercentBins = std::max(1, maxbins / 100);
            if (checkpointFd >= 0 &&
                (int)pendingCheckpointRecords.size() >= onePercentBins) {
                if (write_all(checkpointFd,
                               pendingCheckpointRecords.data(),
                               pendingCheckpointRecords.size() * sizeof(BinRecord)) < 0) {
                    cout << "WARNING: failed to write checkpoint batch ("
                         << pendingCheckpointRecords.size() << " bins)" << endl;
                } else {
                    fsync(checkpointFd);
                }
                pendingCheckpointRecords.clear();
            }

            // Persist progress every CHECKPOINT_PERCENT_STEP percent.
            maybeCheckpoint(false);

            if (g_shutdownRequested) {
                cout << "Shutdown signal received. Saving final checkpoint and telling workers to stop..." << endl;

                // Flush whatever is still buffered so we don't lose the last
                // partial batch on shutdown.
                if (checkpointFd >= 0 && !pendingCheckpointRecords.empty()) {
                    if (write_all(checkpointFd,
                                   pendingCheckpointRecords.data(),
                                   pendingCheckpointRecords.size() * sizeof(BinRecord)) < 0) {
                        cout << "WARNING: failed to write final checkpoint batch ("
                             << pendingCheckpointRecords.size() << " bins)" << endl;
                    } else {
                        fsync(checkpointFd);
                    }
                    pendingCheckpointRecords.clear();
                }

                maybeCheckpoint(true);
                shutdownHandled = true;
                // Tell every worker (including the one we're about to reply to)
                // to stop instead of handing out more work.
                MPI_Send(nullptr, 0, MPI_INT, worker, STOP_TAG, MPI_COMM_WORLD);
                --activeWorkers;
                for (int w = 1; w < size; ++w) {
                    if (w == worker) continue;
                    // Best-effort: these workers are still awaiting a reply to
                    // their previous send; a STOP_TAG send will be picked up
                    // as soon as they post their next receive.
                    MPI_Send(nullptr, 0, MPI_INT, w, STOP_TAG, MPI_COMM_WORLD);
                }
                continue;
            }

            int b = nextPendingBin();
            if (b >= 0) {
                vector<int> nextBinIndices(1, b);
                int nextChunkSize = 1;
                MPI_Send(&nextChunkSize, 1, MPI_INT, worker, SIZE_TAG, MPI_COMM_WORLD);
                MPI_Send(nextBinIndices.data(), nextChunkSize, MPI_INT, worker, BIN_TAG, MPI_COMM_WORLD);
            } else {
                MPI_Send(nullptr, 0, MPI_INT, worker, STOP_TAG, MPI_COMM_WORLD);
                --activeWorkers;
            }
        }

        if (shutdownHandled) {
            cout << "Stopped early due to shutdown signal. Progress is saved in:" << endl;
            cout << "  " << checkpointPath << endl;
            cout << "Re-run with the same runcard/bin range to resume." << endl;
            if (checkpointFd >= 0) close(checkpointFd);
            MPI_Finalize();
            return 0;
        }

        cout << "All bins processed." << endl;

        time_t tableFillEnd = time(0);
        cout << endl;
        cout << "CPU time/bin: "
             << static_cast<double>(tableFillEnd - tableFillStart) / (endingBin - startingBin + 1)
             << " s" << endl;
        cout << "Total time: " << static_cast<double>(tableFillEnd - tableFillStart) / 3600.0 << " h" << endl << endl;

        // Write files only for active selections
        writeTableSnapshots();
        cout << "Selected tables written successfully." << endl;

        // Run finished cleanly: the checkpoint file has served its purpose.
        if (checkpointFd >= 0) close(checkpointFd);
        if (completedBins.size() == static_cast<size_t>(totalBinsThisRun)) {
            if (remove(checkpointPath.c_str()) == 0) {
                cout << "Checkpoint file removed (run complete)." << endl;
            }
        }

        cout << "All done. Bye." << endl;
    } else {
        // Worker node processing loop
        InclusiveDiffractiveCrossSections* idcs = new InclusiveDiffractiveCrossSectionsIntegrals;

        while (true) {
            MPI_Status rstatus;
            int actualChunkSize;
            MPI_Recv(&actualChunkSize, 1, MPI_INT, MASTER, MPI_ANY_TAG, MPI_COMM_WORLD, &rstatus);
            if (rstatus.MPI_TAG == STOP_TAG) break;

            vector<int> binIndex(actualChunkSize);
            MPI_Recv(binIndex.data(), actualChunkSize, MPI_INT, MASTER, BIN_TAG, MPI_COMM_WORLD, &rstatus);

            vector<double> resultData(12 * actualChunkSize);

            for (int i = 0; i < actualChunkSize; ++i) {
                cout << " rank " << rank << " Processing bin [" << binIndex[i] << "/" << endingBin << "]" << endl;

                double out[12];
                int ok = computeBinWithTimeout(
                    binIndex[i],
                    idcs,
                    tableTQQG,
                    out,
                    TIMEOUT_MS
                );

                if (ok == 0) {
                    cout << " rank " << rank << " TIMEOUT or read error on bin " << binIndex[i] << endl;
                } else if (ok < 0) {
                    cout << " rank " << rank << " ERROR (fork/pipe) on bin " << binIndex[i] << endl;
                }

                int idx = i * 12;
                for (int k = 0; k < 12; k++) {
                    resultData[idx++] = out[k];
                }
            }

            MPI_Send(&actualChunkSize, 1, MPI_INT, MASTER, SIZE_TAG, MPI_COMM_WORLD);
            MPI_Send(binIndex.data(), actualChunkSize, MPI_INT, MASTER, BIN_TAG, MPI_COMM_WORLD);
            MPI_Send(resultData.data(), 12 * actualChunkSize, MPI_DOUBLE, MASTER, RESULT_TAG, MPI_COMM_WORLD);
        }

        delete idcs;
    }

    MPI_Finalize();
    return 0;
}