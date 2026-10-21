#include <sstream>  
#include <cstdlib>  
#include <iostream>
#include <limits>
#include <cmath>
#include <iomanip>
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

#include <mpi.h>

#define PR(x) cout << #x << " = " << (x) << endl;

using namespace std;

int main(int argc, char *argv[]) {
    

    MPI_Init(&argc, &argv);
    MPI_Status status;

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    const int MASTER = 0;
    const int SIZE_TAG = 0;       
    const int BIN_TAG  = 1;       // sending bin indices
    const int RESULT_TAG = 2;     // sending computed results
    const int STOP_TAG = 3;       // stop signal
    const int chunksize = 1;      // sending chunk size


    time_t theStartTime = time(0);
    string ctstr(ctime(&theStartTime));
    ctstr.erase(ctstr.size()-1, 1);


    TH1::AddDirectory(false);  // to explicitly delete all new histograms by hand
    TableGeneratorSettings* settings = TableGeneratorSettings::instance();
    //
    //  Check arguments
    //
    char* runcard;
    if (argc != 4) {
        cout << "Usage: tableGeneratorMain runcard startBin endBin" << endl;
        return 2;
    }
    else {
        runcard = argv[1];
        settings->setStartBin(atoi(argv[2]));
        settings->setEndBin(atoi(argv[3]));
    }
    
    cout << "Reading settings from runcard." << endl;
    settings->readSettingsFromFile(runcard);
    settings->consolidateSettings();
    
    //
    // Transfer some of the settings to EventGeneratorSettings:
    //
    EventGeneratorSettings* egsettings = EventGeneratorSettings::instance();
    egsettings->setDipoleModelParameterSet(settings->dipoleModelParameterSet());
    egsettings->setDipoleModelType(settings->dipoleModelType());
    egsettings->setA(settings->A());
    egsettings->setVerboseLevel(settings->verboseLevel());
    egsettings->setApplyPhotonFlux(false);
    
    
    int nBinbeta = settings->betabins();
    int nBinQ2 = settings->Q2bins();
    int nBinW2 = settings->W2bins();
    int nBinz  = settings->zbins();
    double betamin = settings->betamin();
    double betamax = settings->betamax();
    double Q2min =  settings->Q2min();
    double Q2max =  settings->Q2max();
    double Wmin =   settings->Wmin();
    double Wmax =   settings->Wmax();
    double W2min =  Wmin*Wmin;
    double W2max =  Wmax*Wmax;
    double zmin =   settings->zmin();
    double zmax =   settings->zmax();
    unsigned int massA = settings->A();
    DipoleModelType model = settings->dipoleModelType();
    DipoleModelParameterSet pset = settings->dipoleModelParameterSet();
    int startingBin = settings->startBin();
    int endingBin = settings->endBin();
//    int modes = settings->modesToCalculate();
    unsigned char priority = settings->priority();
    bool hotspots = settings->hasSubstructure();
    double fractionOfBinsToFill=settings->fractionOfBinsToFill();
    settings->list();
    
    //
    //  Check bins
    //
    //  Table's bin indices run from 0 ... nbins-1
    //
    int maxbins = nBinbeta*nBinQ2*nBinW2*nBinz;
    if (endingBin >= maxbins) {
        cout << "Warning, given end bin (" << endingBin << ") exceeds the size of the table." << endl;
        cout << "         set to maximum value (" << maxbins << ") now." << endl;
        endingBin = maxbins - 1;
    }
    //
    //   Define all tables. Depending on tableset type some
    //   will not be written but we define them all anyway.
    //
    Table tableTQQ[4];
    Table tableLQQ[4];
    Table tableTQQG[4];
    
    bool logbeta=false, logQ2=true, logW2=true, logz=false, logC=true;
    
    //
    //  Set filenames for the tables.
    //  Be as desciptive as possible. Helps when mass producing tables.
    //
    //  We create all tables and decide later what
    //  gets written and what not.
    //
    string rootfile=settings->rootfile();
    rootfile += "_" + settings->dipoleModelName();
    rootfile += "_" + settings->dipoleModelParameterSetName();
    
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
        
        (void) tableTQQ[i].create(nBinbeta, betamin, betamax,
                                  nBinQ2, Q2min, Q2max,
                                  nBinW2, W2min, W2max,
                                  nBinz,  zmin,  zmax,
                                  logbeta, logQ2, logW2, logz, logC,       // all bools
                                  mean_A, transverse, QQ,
                                  massA, model, pset, i,
                                  filenameTQQ[i].str().c_str(),
                                  priority, hotspots);
        (void) tableLQQ[i].create(nBinbeta, betamin, betamax,
                                  nBinQ2, Q2min, Q2max,
                                  nBinW2, W2min, W2max,
                                  nBinz,  zmin,  zmax,
                                  logbeta, logQ2, logW2, logz, logC,       // all bools
                                  mean_A, longitudinal, QQ,
                                  massA, model, pset, i,
                                  filenameLQQ[i].str().c_str(),
                                  priority, hotspots);
        (void) tableTQQG[i].create(nBinbeta, betamin, betamax,
                                   nBinQ2, Q2min, Q2max,
                                   nBinW2, W2min, W2max,
                                   nBinz,  zmin,  zmax,
                                   logbeta, logQ2, logW2, logz, logC,       // all bools
                                   mean_A, transverse, QQG,
                                   massA, model, pset, i,
                                   filenameTQQG[i].str().c_str(),
                                   priority, hotspots);
        
    }
    cout << "\nAll 3x4 tables created:" << endl;
    for(int i=0; i<4; i++){
        tableTQQ[i].list();
        tableLQQ[i].list();
        tableTQQG[i].list();
    }
    cout << "\nTables have " << maxbins << " bins each.\n" << endl;
    
    cout << "Automatic backup of tables is off.\n" << endl;
    
    //
    // Setup random generator for creating partial tables.
    //
    TRandom3 *random = TableGeneratorSettings::randomGenerator();
    random->SetSeed();

    //
    //   DGLAP Evolution can be speed up by using lookup tables
    //
    DglapEvolution &dglap = DglapEvolution::instance();
    dglap.generateLookupTable(1000, 1000);
    dglap.useLookupTable(true);
    
    //
    //   Print out settings
    //
    cout << endl;
    cout << "Tables will be generated for:" << endl;
    cout << "\tNucleus mass A="<<massA<<endl;
    cout << "\tBins: " << startingBin << "-" << endingBin << endl;
    cout << "\tbeta range: [" << betamin << ", " << betamax << "], " << nBinbeta << " bins." << endl;
    cout << "\tQ2 range: [" << Q2min << ", " << Q2max << "], " << nBinQ2 << " bins." << endl;
    cout << "\tW2 range: [" << W2min << ", " << W2max << "], " << nBinW2 << " bins." << endl;
    cout << "\t z range: [" << zmin << ", " << zmax << "], " << nBinz << " bins." << endl;
    cout << "\tDipole model: " << settings->dipoleModelName() << endl;
    cout << "\tDipole model parameter set: " << settings->dipoleModelParameterSetName() << endl;
    cout << "\tTable set mode: " << settings->tableSetTypeName() << endl;
    cout << endl;
    
    

    //
    //   Calculate contents and fill tables
    //
    //   Note that we fill all tables. What is written
    //   at the end is another story.
    //

    // Mpi specific part 

    if(rank == MASTER) {
        cout << "Start filling tables:" << endl;
        time_t tableFillStart = time(0);

        int nextBin = startingBin;
        int activeWorkers = 0;

        vector<int> binIndices;
        int actualChunkSize;

        // Initial distribution
        for(int worker = 1; worker < size && nextBin <= endingBin; ++worker) {
            actualChunkSize = min(chunksize, endingBin - nextBin + 1);
            binIndices.resize(actualChunkSize);
            for(int i=0; i<actualChunkSize; ++i) binIndices[i] = nextBin++;

            MPI_Send(&actualChunkSize, 1, MPI_INT, worker, SIZE_TAG, MPI_COMM_WORLD);
            MPI_Send(binIndices.data(), actualChunkSize, MPI_INT, worker, BIN_TAG, MPI_COMM_WORLD);
            ++activeWorkers;
        }

        while(activeWorkers > 0) {
            MPI_Status status;

            // Receive bin indices processed
            MPI_Recv(binIndices.data(), actualChunkSize, MPI_INT, MPI_ANY_SOURCE, BIN_TAG, MPI_COMM_WORLD, &status);
            int worker = status.MPI_SOURCE;

            // Receive results
            int valuerec = 12 * actualChunkSize;
            double resultData[valuerec];
            MPI_Recv(resultData, valuerec, MPI_DOUBLE, worker, RESULT_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            
            // Fill tables
            for(int i = 0; i < actualChunkSize; ++i) {
                int bin = binIndices[i];
                int idx = i * 12; // 12 values per bin
                for(int iq=0; iq<4; ++iq){
                    tableTQQ[iq].fill(bin, resultData[idx++], 0);
                    tableLQQ[iq].fill(bin, resultData[idx++], 0);
                    tableTQQG[iq].fill(bin, resultData[idx++], 0);
                }
            }

            // Send next chunk or stop
            if(nextBin <= endingBin) {
                actualChunkSize = min(chunksize, endingBin - nextBin + 1);
                binIndices.resize(actualChunkSize);
                for(int i=0; i<actualChunkSize; ++i) binIndices[i] = nextBin++;

                MPI_Send(&actualChunkSize, 1, MPI_INT, worker, SIZE_TAG, MPI_COMM_WORLD);
                MPI_Send(binIndices.data(), actualChunkSize, MPI_INT, worker, BIN_TAG, MPI_COMM_WORLD);
            } else {
                MPI_Send(nullptr, 0, MPI_INT, worker, STOP_TAG, MPI_COMM_WORLD);
                --activeWorkers;
            }
        }

        cout << "All bins processed." << endl;

        time_t tableFillEnd = time(0);
        cout << endl;
        cout << "CPU time/bin: "
        <<  static_cast<double>(tableFillEnd-tableFillStart)/(endingBin-startingBin+1)
        << " s" << endl;
        cout << "Total time: " << static_cast<double>(tableFillEnd-tableFillStart)/60./60. << " h" << endl << endl;

        for(int i=0; i<4; i++){
            tableTQQ[i].write();
            tableLQQ[i].write();
            tableTQQG[i].write();
        }
        cout << "All tables written" << endl;

        cout << "All done. Bye." << endl;
    }


    else {
        InclusiveDiffractiveCrossSections* idcs = new InclusiveDiffractiveCrossSectionsIntegrals;

        while(true) {
            MPI_Status status;
            int actualChunkSize;

            // Receive chunk size
            MPI_Recv(&actualChunkSize, 1, MPI_INT, MASTER, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            if(status.MPI_TAG == STOP_TAG) break;

            vector<int> binIndex(actualChunkSize);
            MPI_Recv(binIndex.data(), actualChunkSize, MPI_INT, MASTER, BIN_TAG, MPI_COMM_WORLD, &status);

            double resultData[12 * actualChunkSize];

            for(int i=0; i<actualChunkSize; ++i){
                cout<<" rank "<<rank<<" Processing bin ["<<binIndex[i]<<"/"<<endingBin<<"]"<<endl;
                double Q2, W2, z, beta;
                tableTQQ[0].binCenter(binIndex[i], Q2, W2, beta, z);
                time_t t1 = time(0);
                idcs->dsigdbetadQ2dW2dz_total(beta, Q2, W2, z, transverse);
                time_t t2 = time(0);  cout << " time taken to calculate transverse = "<< abs(t2 -t1)<<endl;
                idcs->dsigdbetadQ2dW2dz_total(beta, Q2, W2, z, longitudinal);
                time_t t3 = time(0);  cout << " time taken to calculate longitudinal = "<< abs(t3 -t2)<<endl;
                idcs->dsigdbetadQ2dW2dz_qqg(beta, Q2, W2, z);
                time_t t4 = time(0); cout << " time taken to calculate qqg = "<< abs(t4 -t3)<<endl;


                int idx = i*12;
                for(int iq=0; iq<4; ++iq){
                    resultData[idx++] = idcs->dsigdbetadQ2dWdz_T_total()[iq];
                    resultData[idx++] = idcs->dsigdbetadQ2dWdz_L_total()[iq];
                    resultData[idx++] = idcs->dsigdbetadQ2dWdz_T_qqg()[iq];
                }
            }

            // Send back results
            MPI_Send(binIndex.data(), actualChunkSize, MPI_INT, MASTER, BIN_TAG, MPI_COMM_WORLD);
            MPI_Send(resultData, 12 * actualChunkSize, MPI_DOUBLE, MASTER, RESULT_TAG, MPI_COMM_WORLD);
        }

       
    }


    

    
    MPI_Finalize();
    return 0;

}
