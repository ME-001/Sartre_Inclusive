
//
// sf_mpi.cpp
// MPI-ready version of sf.cpp
//
// Build (example, adjust to your ROOT/Sartre/MPI setup):
//   mpicxx -O2 -std=c++17 sf_mpi.cpp -o sf_mpi \
//       $(root-config --cflags --libs) \
//       -I$SARTRE_INCLUDE -L$SARTRE_LIB -lSartre \
//       -lMathMore
//
// Run:
//   mpirun -np <N> ./sf_mpi runcard.ini <Q2>
//

#include <iostream>
#include <sstream>
#include <string>
#include <cmath>

#include "TTree.h"
#include "TFile.h"
#include "TString.h"
#include "SartreInclusiveDiffraction.h"
#include "DipoleModelParameters.h"
#include "TGenPhaseSpace.h"
#include "Settings.h"
#include "TH1D.h"
#include "TROOT.h"
#include "TLorentzVector.h"
#include "TClonesArray.h"
#include "InclusiveDiffractiveCrossSections.h"
#include "DglapEvolution.h"
#include "Kinematics.h"
#include "TableCollection.h"
#include "Table.h"
#include "InclusiveDiffractiveCrossSectionsFromTables.h"


#include "Math/GaussLegendreIntegrator.h"
#include "Math/Integrator.h"
#include "Math/WrappedTF1.h"

#include <mpi.h>

#define PR(x) cout << #x << " = " << (x) << endl;

using namespace std;

// ---------------------------------------------------------------------
// Global cross-section object used by the TF1 wrapper functions below.
// Every MPI rank builds its own instance during setup, so this is safe
// even though it is a global: each process has its own address space.
// ---------------------------------------------------------------------
InclusiveDiffractiveCrossSections* myICS = 0;
TableCollection* tc = 0;

double mFT(double* var, double* par)
{
    double beta = par[0];
    double Q2   = par[1];
    double W2   = par[2];
    double z    = var[0];
    return myICS->dsigmadbetadz_T(beta, Q2, W2, z);
}

double mFL(double* var, double* par)
{
    double beta = par[0];
    double Q2   = par[1];
    double W2   = par[2];
    double z    = var[0];
    return myICS->dsigmadbetadz_L(beta, Q2, W2, z);
}

double mFQQG(double* var, double* par)
{
    double beta = par[0];
    double Q2   = par[1];
    double W2   = par[2];
    double z    = var[0];
    return myICS->dsigmadbetadz_QQG(beta, Q2, W2, z); // nb
}

struct rootSartreEvent {
    double t;
    double beta;
    double Q2;
    double x;
    double s;
    double y;
    double W;
    double z;
    double MX;
    double xpom;
    double r;
    double rs;
    double QS;
    double b;
    int    iEvent;
    int    pol;
    int    dmode;
};

rootSartreEvent myRootSartreEvent;
void randomlyReverseBeams(Event*);

// ---------------------------------------------------------------------
// Per-bin physics calculation, shared by workers (and by rank 0 if it
// also does work). Loops over 4 quark flavors, as in the serial code.
// Fills result[0..2] = {csT, csL, csqqg}.
// ---------------------------------------------------------------------
void computeBin(int binIndex, TH1D& FT, double Q2, double xpom,
                 TF1& fT, TF1& fL, TF1& fQQG,
                 ROOT::Math::GaussIntegrator& giT,
                 ROOT::Math::GaussIntegrator& giL,
                 ROOT::Math::GaussIntegrator& giQQG,
                 double result[3])
{
    double beta = FT.GetBinCenter(binIndex);
    double x    = beta * xpom;
    double W2   = Kinematics::W2(Q2, x);

    double prefactor = beta * Q2 / (4 * M_PI * M_PI * alpha_em) / 1e7 / hbarc2;

    double csT = 0, csL = 0, csqqg = 0;

    for (int quark = 0; quark < 4; quark++) {
        myICS->setQuarkIndex(quark);

        fT.SetParameter(0, beta);
        fL.SetParameter(0, beta);
        fQQG.SetParameter(0, beta);
        fT.SetParameter(1, Q2);
        fL.SetParameter(1, Q2);
        fQQG.SetParameter(1, Q2);
        fT.SetParameter(2, W2);
        fL.SetParameter(2, W2);
        fQQG.SetParameter(2, W2);

        csT   += giT.Integral(0.001, 0.999);
        csL   += giL.Integral(0.001, 0.999);
        csqqg += giQQG.Integral(beta, 0.999);
    }

    csT   *= prefactor;
    csL   *= prefactor;
    csqqg *= prefactor;

    if (std::isnan(csT))   csT   = 0;
    if (std::isnan(csL))   csL   = 0;
    if (std::isnan(csqqg)) csqqg = 0;

    result[0] = csT;
    result[1] = csL;
    result[2] = csqqg;
}

int main(int argc, char* argv[])
{
    MPI_Init(&argc, &argv);
    MPI_Status status;

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    const int MASTER    = 0;
    const int BIN_TAG    = 1;
    const int RESULT_TAG = 2;
    const int STOP_TAG   = 3;

    //
    //  Check command line arguments
    //
    if (argc < 3) {
        if (rank == MASTER)
            cout << "Usage:  " << argv[0] << " runcard Q2 [rootfile]" << endl;
        MPI_Finalize();
        return 2;
    }
    string runcard = argv[1];
    double Q2      = stod(argv[2]);

    //
    //  Create the generator and initialize it.
    //  Every rank does this independently (separate address spaces).
    //
    SartreInclusiveDiffraction sartre;
    bool ok = sartre.init(runcard);
    if (!ok) {
        cerr << "[rank " << rank << "] Initialization of sartre failed." << endl;
        MPI_Finalize();
        return 1;
    }

    EventGeneratorSettings* settings = sartre.runSettings();

    //
    //  ROOT file name override, if given.
    //
    string rootfile;
    if (argc == 4) {
        rootfile = argv[3];
        settings->setRootfile(argv[3]);
    }
    else {
        rootfile = settings->rootfile();
    }

    if (rank == MASTER) settings->list();

    int bin = 100;

    string mode;
    string tableBin;
    double xpom = 1e-3;

    double min_Rng = 0.001;
    double max_Rng = 0.9;
    TH1D FT("SF_T", "sf_T", bin, min_Rng, max_Rng);
    TH1D FL("SF_L", "sf_L", bin, min_Rng, max_Rng);
    TH1D FQQG("SF_QQG", "sf_qqg", bin, min_Rng, max_Rng);
    TH1D FTot("SF_tot", "sf_tot", bin, min_Rng, max_Rng);

    myICS = sartre.mCrossSection;
    bool model = sartre.mUseCrossSectionModel;
    bool table = sartre.mUseCrossSectionTables;
    int a = sartre.GetA();

    if (model) {
        DipoleModelType dipoleType       = DipoleModelType::bSat;              // TODO: confirm
        DipoleModelParameterSet dipoleSet = DipoleModelParameterSet::STU;      // TODO: confirm

        SurrogateGuardGridParams gridParams =
            InclusiveDiffractionCrossSectionsFromModel::gridParamsFromSettings();

        myICS = new InclusiveDiffractionCrossSectionsFromModel(a, dipoleType, dipoleSet, gridParams);
        mode = "model";
        tableBin = "model";
    }
    else if (table) {
        TableCollection* localTc = sartre.mTableCollection;
        myICS = new InclusiveDiffractionCrossSectionsFromTables(localTc);
        mode = "table";
        tableBin = "table";
    }
    else {
        myICS = new InclusiveDiffractiveCrossSectionsIntegrals();
        mode = "theory";
        tableBin = "";
    }

    TF1 fT("fT", mFT, 0.001, 0.999, 3);
    TF1 fL("fL", mFL, 0.001, 0.999, 3);
    TF1 fQQG("fQQG", mFQQG, 0.001, 0.999, 3);

    fT.SetParameter(1, Q2);
    fL.SetParameter(1, Q2);
    fQQG.SetParameter(1, Q2);

    ROOT::Math::WrappedTF1 wfT(fT);
    ROOT::Math::GaussIntegrator giT;

    ROOT::Math::WrappedTF1 wfL(fL);
    ROOT::Math::GaussIntegrator giL;

    ROOT::Math::WrappedTF1 wfQQG(fQQG);
    ROOT::Math::GaussIntegrator giQQG;

    giT.SetFunction(wfT);
    giL.SetFunction(wfL);
    giQQG.SetFunction(wfQQG);

    giT.SetAbsTolerance(0.);
    giT.SetRelTolerance(1e-6);

    giL.SetAbsTolerance(0.);
    giL.SetRelTolerance(1e-6);

    giQQG.SetAbsTolerance(0.);
    giQQG.SetRelTolerance(1e-6);

    // -------------------------------------------------------------
    //  MPI master / worker split
    // -------------------------------------------------------------
    if (size == 1) {
        // No workers available: rank 0 does everything itself.
        for (int i = 1; i <= bin; i++) {
            cout << "Filling bin# " << i << endl;
            double result[3];
            computeBin(i, FT, Q2, xpom, fT, fL, fQQG, giT, giL, giQQG, result);
            double cstot = result[0] + result[1] + result[2];
            FT.SetBinContent(i, result[0]);
            FL.SetBinContent(i, result[1]);
            FQQG.SetBinContent(i, result[2]);
            FTot.SetBinContent(i, cstot);
        }
    }
    else if (rank == MASTER) {
        cout << " Start filling bins" << endl;
        int nextBin = 1;
        int activeWorkers = 0;

        // Prime every worker with an initial bin.
        for (int worker = 1; worker < size && nextBin <= bin; worker++) {
            MPI_Send(&nextBin, 1, MPI_INT, worker, BIN_TAG, MPI_COMM_WORLD);
            ++activeWorkers;
            ++nextBin;
        }

        while (activeWorkers > 0) {
            int binIndex;
            MPI_Recv(&binIndex, 1, MPI_INT, MPI_ANY_SOURCE, BIN_TAG, MPI_COMM_WORLD, &status);
            int worker = status.MPI_SOURCE;

            double result[3];
            MPI_Recv(result, 3, MPI_DOUBLE, worker, RESULT_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            double cstot = result[0] + result[1] + result[2];
            FT.SetBinContent(binIndex, result[0]);
            FL.SetBinContent(binIndex, result[1]);
            FQQG.SetBinContent(binIndex, result[2]);
            FTot.SetBinContent(binIndex, cstot);

            cout << " received bin " << binIndex << " from rank " << worker << endl;

            if (nextBin <= bin) {
                MPI_Send(&nextBin, 1, MPI_INT, worker, BIN_TAG, MPI_COMM_WORLD);
                ++nextBin;
            }
            else {
                MPI_Send(nullptr, 0, MPI_INT, worker, STOP_TAG, MPI_COMM_WORLD);
                --activeWorkers;
            }
        }
    }
    else {
        // Worker rank: loop receiving bin indices until told to stop.
        while (true) {
            int binIndex;
            MPI_Recv(&binIndex, 1, MPI_INT, MASTER, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            if (status.MPI_TAG == STOP_TAG) break;

            cout << "[rank " << rank << "] calculating bin: " << binIndex << endl;

            double result[3];
            computeBin(binIndex, FT, Q2, xpom, fT, fL, fQQG, giT, giL, giQQG, result);

            cout << "[rank " << rank << "] calculated bin: " << binIndex << endl;

            MPI_Send(&binIndex, 1, MPI_INT, MASTER, BIN_TAG, MPI_COMM_WORLD);
            MPI_Send(result, 3, MPI_DOUBLE, MASTER, RESULT_TAG, MPI_COMM_WORLD);
        }
    }

    // -------------------------------------------------------------
    //  Only the master (or the lone rank in a single-process run)
    //  writes the output file.
    // -------------------------------------------------------------
    if (rank == MASTER) {
        ostringstream name;
        name << "SFvbeta_" << mode << "_A" << a
             << "_Q2_" << Q2
             << "_xpom_" << xpom
             << "_" << tableBin
             << "_EIC_" << bin;
        TString filename = name.str();

        TFile* hfile = new TFile(filename + ".root", "RECREATE");
        FT.Write();
        FL.Write();
        FQQG.Write();
        FTot.Write();
        hfile->Write();
        delete hfile;

        cout << filename + ".root" << " has been created" << endl;
    }

    MPI_Finalize();
    return 0;
}
