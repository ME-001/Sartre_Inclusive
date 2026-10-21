#include <sstream>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <cmath>
#include <iomanip>
#include <string>
#include <vector>
#include <array>
#include <algorithm>
#include <memory>

#include "mpi.h"

#include "TROOT.h"
#include "TH1D.h"
#include "TFile.h"
#include "Nucleus.h"
#include "Constants.h"
#include "Table.h"
#include "TableGeneratorSettings.h"
#include "Enumerations.h"
#include "Version.h"
#include "TRandom3.h"

// NOTE: pulls LibTorch in via SurrogateModelCollection.h -- keep this as
// close to the top of the include list as InclusiveDiffractiveCrossSecitonfromModel.h
// itself requires (see that header's own include-order warning).
#include "InclusiveDiffractiveCrossSectionsFromModel.h"

#define PR(x) cout << #x << " = " << (x) << endl;

using namespace std;

//==============================================================================
//  Internal constants
//==============================================================================
static constexpr int TAG_WORK   = 1;
static constexpr int TAG_RESULT = 2;
static constexpr int SENTINEL   = -1;
static constexpr int N_TYPES    = 3;   // TQQ, LQQ, TQQG
static constexpr int N_IQ       = 4;   // quark species
static constexpr int RESULT_LEN = N_IQ * N_TYPES;  // 12

// table_type indices, matching the payload layout below: idx = iq*3 + type.
static constexpr int TABLE_TYPE_TQQ  = 0;
static constexpr int TABLE_TYPE_LQQ  = 1;
static constexpr int TABLE_TYPE_TQQG = 2;

//==============================================================================
//  GridParams — broadcast from master to all workers
//==============================================================================
struct GridParams {
    int    nBinbeta, nBinQ2, nBinW2, nBinz;
    double betamin,  betamax;
    double Q2min,    Q2max;
    double W2min,    W2max;
    double zmin,     zmax;
    unsigned int massA;
    int    dipmodel, pset, priority;
    int    hotspots;
    double fractionOfBinsToFill;
    int    logbeta, logQ2, logW2, logz, logC; // 0/1 flags
};

//==============================================================================
//  workerLoop — load the guarded surrogate cross section once, evaluate +
//  guard per bin (both now handled inside InclusiveDiffractionCrossSectionsFromModel),
//  send results back to master.
//
//  All the physics-guard machinery (hard phase-space gates, danger-zone
//  spike probing, stepQ2/stepZ/stepW2 helpers, GuardGridParams, etc.) that
//  used to live in this file has been removed: it is now owned by
//  InclusiveDiffractionCrossSectionsFromModel::dsigmadbetadz_T/_L/_QQG (via
//  evaluateGuarded()), so this file just calls those and forwards the
//  results. This keeps table-generation time and runtime guard logic in a
//  single place instead of two copies that can drift apart.
//==============================================================================
static void workerLoop(int rank, const GridParams& gp)
{
    cout << "[rank " << rank << "] Constructing guarded surrogate cross section..." << endl;

    // ---- construction, following the reference initialization pattern ----
    const int A = static_cast<int>(gp.massA);
    DipoleModelType dipoleType = static_cast<DipoleModelType>(gp.dipmodel);
    DipoleModelParameterSet dipoleSet = static_cast<DipoleModelParameterSet>(gp.pset);

    SurrogateGuardGridParams gridParams = InclusiveDiffractionCrossSectionsFromModel::gridParamsFromSettings();

    std::unique_ptr<InclusiveDiffractionCrossSectionsFromModel> crossSection(
        new InclusiveDiffractionCrossSectionsFromModel(A, dipoleType, dipoleSet, gridParams));

    cout << "[rank " << rank << "] Using surrogate model for cross-section calculations." << endl;

    ostringstream dummy;
    dummy << "/dev/null_rank" << rank << ".root";
    Table refTable;
    refTable.create(
        gp.nBinbeta, gp.betamin, gp.betamax,
        gp.nBinQ2,   gp.Q2min,   gp.Q2max,
        gp.nBinW2,   gp.W2min,   gp.W2max,
        gp.nBinz,    gp.zmin,    gp.zmax,
        gp.logbeta!=0, gp.logQ2!=0, gp.logW2!=0, gp.logz!=0, gp.logC!=0,
        mean_A, transverse, QQ,
        gp.massA,
        static_cast<DipoleModelType>(gp.dipmodel),
        static_cast<DipoleModelParameterSet>(gp.pset),
        0,
        dummy.str().c_str(),
        static_cast<unsigned char>(gp.priority),
        gp.hotspots != 0);

    double payload[RESULT_LEN + 1];

    double nProcessed  = 0.0;
    double nZeroedTQQ  = 0.0;
    double nZeroedLQQ  = 0.0;
    double nZeroedTQQG = 0.0;

    while (true) {
        int binIdx;
        MPI_Recv(&binIdx, 1, MPI_INT, 0, TAG_WORK, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        if (binIdx == SENTINEL) break;

        payload[0] = static_cast<double>(binIdx);

        double Q2, W2, beta, z;
        refTable.binCenter(binIdx, Q2, W2, beta, z);
        ++nProcessed;

        for (int iq = 0; iq < N_IQ; ++iq) {
            // TODO: confirm this is the correct call on your build to select
            // the quark flavor before evaluating -- InclusiveDiffractiveCrossSections
            // (the base class) is documented as owning mQuarkIndex, but the
            // setter name wasn't in the excerpt provided. Adjust if your
            // header spells it differently (e.g. setQuark(), setFlavor()).
            crossSection->setQuarkIndex(static_cast<unsigned int>(iq));

            double valTQQ  = crossSection->dsigmadbetadz_T(beta, Q2, W2, z);
            double valLQQ  = crossSection->dsigmadbetadz_L(beta, Q2, W2, z);
            double valTQQG = crossSection->dsigmadbetadz_QQG(beta, Q2, W2, z);

            payload[1 + iq * N_TYPES + TABLE_TYPE_TQQ]  = (isnan(valTQQ) || isinf(valTQQ)) ? 0.0 : valTQQ;
            payload[1 + iq * N_TYPES + TABLE_TYPE_LQQ]  = (isnan(valLQQ) || isinf(valLQQ)) ? 0.0 : valLQQ;
            payload[1 + iq * N_TYPES + TABLE_TYPE_TQQG] = (isnan(valTQQG) || isinf(valTQQG)) ? 0.0 : valTQQG;

            // Guard reasons (zeroed vs. spike-corrected) are no longer
            // surfaced by evaluateGuarded()/dsigmadbetadz_* (their internal
            // per-channel counters are currently commented out on the
            // provided source), so the only thing we can still tally here
            // is "came back exactly zero", which is a proxy for
            // out-of-phase-space rather than a true guard-reason breakdown.
            if (valTQQ  == 0.0) nZeroedTQQ  += 1.0;
            if (valLQQ  == 0.0) nZeroedLQQ  += 1.0;
            if (valTQQG == 0.0) nZeroedTQQG += 1.0;
        }

        MPI_Send(payload, RESULT_LEN + 1, MPI_DOUBLE, 0, TAG_RESULT, MPI_COMM_WORLD);
    }

    double countsLocal[4]  = { nProcessed, nZeroedTQQ, nZeroedLQQ, nZeroedTQQG };
    double countsGlobal[4] = { 0.0, 0.0, 0.0, 0.0 };
    MPI_Reduce(countsLocal, countsGlobal, 4, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
}

//==============================================================================
//  masterLoop — dispatch bins and assign results
//==============================================================================
static void masterLoop(int worldSize, const vector<int>& binsToFill,
                        Table tableTQQ[4], Table tableLQQ[4], Table tableTQQG[4])
{
    int nWorkers = worldSize - 1;
    size_t totalBins = binsToFill.size();
    size_t nextIdx = 0;
    int pending = 0;

    // Progress ticker cadence: print roughly once every 1% of totalBins
    // completed (clamped to at least 1, so tiny runs still print). Lower
    // the divisor (e.g. totalBins/20) for more frequent updates on long
    // runs, or raise it to reduce log spam on very large ones.
    size_t nShow = std::max<size_t>(1, totalBins / 100);
    size_t doneCount = 0;

    double payload[RESULT_LEN + 1];
    MPI_Status status;

    // Hands the next unfilled bin index to worker `dest` and marks it
    // in-flight (pending). Called once per idle worker at startup, and
    // again each time a worker's result comes back with bins left to hand out.
    auto sendNext = [&](int dest) {
        int binIdx = binsToFill[nextIdx++];
        MPI_Send(&binIdx, 1, MPI_INT, dest, TAG_WORK, MPI_COMM_WORLD);
        ++pending;
    };

    // Prime every worker with one bin each to start with.
    for (int w = 1; w <= nWorkers && nextIdx < totalBins; ++w)
        sendNext(w);

    // Classic master-worker pull loop: as each result comes back, record
    // it, print progress, then immediately re-feed that worker (if bins
    // remain) so no worker sits idle while others are still computing.
    while (pending > 0) {
        MPI_Recv(payload, RESULT_LEN + 1, MPI_DOUBLE,
                 MPI_ANY_SOURCE, TAG_RESULT, MPI_COMM_WORLD, &status);
        --pending;
        ++doneCount;

        int doneBin = static_cast<int>(payload[0]);
        for (int iq = 0; iq < N_IQ; ++iq) {
            double valTQQ  = payload[1 + iq * N_TYPES + TABLE_TYPE_TQQ];
            double valLQQ  = payload[1 + iq * N_TYPES + TABLE_TYPE_LQQ];
            double valQQG  = payload[1 + iq * N_TYPES + TABLE_TYPE_TQQG];
            tableTQQ[iq].fill(doneBin, valTQQ, 0);
            tableLQQ[iq].fill(doneBin, valLQQ, 0);
            tableTQQG[iq].fill(doneBin, valQQG, 0);
        }

        // Progress printout: "filled <done> / <total>" plus which bin and
        // which worker rank just finished it, so a stall can be traced to
        // a specific rank. Gated by nShow so this doesn't fire on every
        // single bin (which would flood stdout on large grids); always
        // fires on the very last bin so the log ends at 100%.
        if (doneCount % nShow == 0 || doneCount == totalBins)
            cout << "  filled " << doneCount << " / " << totalBins
                 << "  (bin " << doneBin << ", rank " << status.MPI_SOURCE << ")\n";

        if (nextIdx < totalBins)
            sendNext(status.MPI_SOURCE);
    }

    // All bins dispatched and collected -- tell every worker to exit its
    // receive loop.
    int sentinel = SENTINEL;
    for (int w = 1; w <= nWorkers; ++w)
        MPI_Send(&sentinel, 1, MPI_INT, w, TAG_WORK, MPI_COMM_WORLD);
}

//==============================================================================
//  main
//==============================================================================
int main(int argc, char *argv[]) {

    MPI_Init(&argc, &argv);
    int rank, worldSize;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &worldSize);

    if (worldSize < 2) {
        if (rank == 0) cerr << "ERROR: need at least 2 MPI ranks (1 master + >=1 worker).\n";
        MPI_Finalize();
        return 1;
    }

    if (rank == 0) {
        time_t theStartTime = time(0);
        string ctstr(ctime(&theStartTime));
        ctstr.erase(ctstr.size()-1, 1);
        cout << "/========================================================================\\" << endl;
        cout << "|                                                                        |" << endl;
        cout << "|  Sartre, Version " << setw(54) << left << VERSION << right << '|' << endl;
        cout << "|                                                                        |" << endl;
        cout << "|  Table Generator -- SURROGATE MODE (MPI, physics-guarded)              |" << endl;
        cout << "|  Guarding now delegated to InclusiveDiffractionCrossSectionsFromModel  |" << endl;
        cout << "|  (dsigmadbetadz_T / _L / _QQG), so it matches runtime evaluation.       |" << endl;
        cout << "|                                                                        |" << endl;
        cout << "\\========================================================================/" << endl;
    }

    GridParams gp{};

    if (rank == 0) {
        if (argc != 2) {
            cerr << "Usage: mpirun -np <N> tableGeneratorMainSurrogate_mpi runcard" << endl;
            MPI_Abort(MPI_COMM_WORLD, 2);
        }
        char* runcard = argv[1];

        TH1::AddDirectory(false);
        TableGeneratorSettings* settings = TableGeneratorSettings::instance();

        cout << "Reading settings from runcard." << endl;
        settings->readSettingsFromFile(runcard);
        settings->consolidateSettings();

        gp.nBinbeta = settings->betabins();
        gp.nBinQ2   = settings->Q2bins();
        gp.nBinW2   = settings->W2bins();
        gp.nBinz    = settings->zbins();
        gp.betamin  = settings->betamin();
        gp.betamax  = settings->betamax();
        gp.Q2min    = settings->Q2min();
        gp.Q2max    = settings->Q2max();
        double Wmin = settings->Wmin();
        double Wmax = settings->Wmax();
        gp.W2min    = Wmin*Wmin;
        gp.W2max    = Wmax*Wmax;
        gp.zmin     = settings->zmin();
        gp.zmax     = settings->zmax();
        gp.massA    = settings->A();
        gp.dipmodel = static_cast<int>(settings->dipoleModelType());
        gp.pset     = static_cast<int>(settings->dipoleModelParameterSet());
        gp.priority = static_cast<int>(settings->priority());
        gp.hotspots = settings->hasSubstructure() ? 1 : 0;
        gp.fractionOfBinsToFill = settings->fractionOfBinsToFill();

        int maxbinsTmp = gp.nBinbeta*gp.nBinQ2*gp.nBinW2*gp.nBinz;
        settings->setStartBin(0);
        settings->setEndBin(maxbinsTmp - 1);

        gp.logbeta = 0;
        gp.logQ2   = 1;
        gp.logW2   = 1;
        gp.logz    = 0;
        gp.logC    = 1;

        settings->list();
    }

    MPI_Bcast(&gp, sizeof(GridParams), MPI_BYTE, 0, MPI_COMM_WORLD);

    int maxbins = gp.nBinbeta*gp.nBinQ2*gp.nBinW2*gp.nBinz;

    if (rank != 0) {
        workerLoop(rank, gp);
        MPI_Finalize();
        return 0;
    }

    TableGeneratorSettings* settings = TableGeneratorSettings::instance();
    unsigned char priority = static_cast<unsigned char>(gp.priority);
    bool hotspots = gp.hotspots != 0;

    Table tableTQQ[4];
    Table tableLQQ[4];
    Table tableTQQG[4];

    bool logbeta = gp.logbeta!=0, logQ2 = gp.logQ2!=0, logW2 = gp.logW2!=0,
         logz = gp.logz!=0, logC = gp.logC!=0;

    string rootfile = settings->rootfile();
    rootfile += "_" + settings->dipoleModelName();
    rootfile += "_" + settings->dipoleModelParameterSetName();
    rootfile += "_surrogate";

    int startingBin = 0;
    int endingBin = maxbins - 1;

    ostringstream filenameTQQ[4], filenameLQQ[4], filenameTQQG[4];
    for(int i=0; i<4; i++){
        filenameTQQ[i].str("");
        filenameTQQ[i] << rootfile << "_q" << i << "_bin"
        << startingBin << "-" << endingBin <<"_TQQ.root";

        filenameLQQ[i].str("");
        filenameLQQ[i] << rootfile << "_q" << i << "_bin"
        << startingBin << "-" << endingBin << "_LQQ.root";

        filenameTQQG[i].str("");
        filenameTQQG[i] << rootfile << "_q" << i << "_bin"
        << startingBin << "-" << endingBin << "_TQQG.root";

        (void) tableTQQ[i].create(gp.nBinbeta, gp.betamin, gp.betamax,
                                  gp.nBinQ2, gp.Q2min, gp.Q2max,
                                  gp.nBinW2, gp.W2min, gp.W2max,
                                  gp.nBinz,  gp.zmin,  gp.zmax,
                                  logbeta, logQ2, logW2, logz, logC,
                                  mean_A, transverse, QQ,
                                  gp.massA, static_cast<DipoleModelType>(gp.dipmodel),
                                  static_cast<DipoleModelParameterSet>(gp.pset), i,
                                  filenameTQQ[i].str().c_str(),
                                  priority, hotspots);
        (void) tableLQQ[i].create(gp.nBinbeta, gp.betamin, gp.betamax,
                                  gp.nBinQ2, gp.Q2min, gp.Q2max,
                                  gp.nBinW2, gp.W2min, gp.W2max,
                                  gp.nBinz,  gp.zmin,  gp.zmax,
                                  logbeta, logQ2, logW2, logz, logC,
                                  mean_A, longitudinal, QQ,
                                  gp.massA, static_cast<DipoleModelType>(gp.dipmodel),
                                  static_cast<DipoleModelParameterSet>(gp.pset), i,
                                  filenameLQQ[i].str().c_str(),
                                  priority, hotspots);
        (void) tableTQQG[i].create(gp.nBinbeta, gp.betamin, gp.betamax,
                                   gp.nBinQ2, gp.Q2min, gp.Q2max,
                                   gp.nBinW2, gp.W2min, gp.W2max,
                                   gp.nBinz,  gp.zmin,  gp.zmax,
                                   logbeta, logQ2, logW2, logz, logC,
                                   mean_A, transverse, QQG,
                                   gp.massA, static_cast<DipoleModelType>(gp.dipmodel),
                                   static_cast<DipoleModelParameterSet>(gp.pset), i,
                                   filenameTQQG[i].str().c_str(),
                                   priority, hotspots);
    }
    cout << "\nAll 3x4 tables created." << endl;

    TRandom3 *random = TableGeneratorSettings::randomGenerator();
    random->SetSeed();

    // gp.fractionOfBinsToFill is applied as a double independent draw
    // (same as the single-threaded generator) -- a bin only makes it into
    // binsToFill if it survives BOTH random draws, so the effective keep
    // probability is fractionOfBinsToFill^2, not fractionOfBinsToFill.
    vector<int> binsToFill;
    binsToFill.reserve(maxbins);
    for (int i = startingBin; i <= endingBin; ++i) {
        if (gp.fractionOfBinsToFill <= random->Uniform()) continue;
        if (gp.fractionOfBinsToFill <= random->Uniform()) continue;
        binsToFill.push_back(i);
    }
    // Total bin count selected for this run, out of the full grid size --
    // printed once, before any work is dispatched.
    cout << "\tBins selected to fill: " << binsToFill.size() << " / " << maxbins << endl;

    cout << "Start filling tables:" << endl;
    time_t tableFillStart = time(0);

    // Live progress ("filled X / Y") is printed from inside this call --
    // see masterLoop()'s nShow-gated cout.
    masterLoop(worldSize, binsToFill, tableTQQ, tableLQQ, tableTQQG);

    time_t tableFillEnd = time(0);

    // Wall-clock time for the whole fill, in hours.
    cout << "Total time: " << static_cast<double>(tableFillEnd-tableFillStart)/3600.0 << " h" << endl;

    double countsLocal[4]  = { 0.0, 0.0, 0.0, 0.0 };
    double countsGlobal[4] = { 0.0, 0.0, 0.0, 0.0 };
    MPI_Reduce(countsLocal, countsGlobal, 4, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

    cout << "Physics guard summary (guarding now lives inside" << endl;
    cout << "InclusiveDiffractionCrossSectionsFromModel; 'zeroed' below is a" << endl;
    cout << "proxy -- a returned-exactly-zero count, not a full guard-reason" << endl;
    cout << "breakdown, since that detail isn't surfaced through dsigmadbetadz_*):" << endl;
    cout << "\tBins processed:              " << countsGlobal[0] << endl;
    cout << "\tTQQ   returned zero:          " << countsGlobal[1] << endl;
    cout << "\tLQQ   returned zero:          " << countsGlobal[2] << endl;
    cout << "\tTQQG  returned zero:          " << countsGlobal[3] << endl;

    for(int i=0; i<4; i++){
        tableTQQ[i].write();
        tableLQQ[i].write();
        tableTQQG[i].write();
    }
    cout << "All tables written. Bye." << endl;

    MPI_Finalize();
    return 0;
}