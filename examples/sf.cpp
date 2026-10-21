
#include <iostream>
#include <cmath>
#include "TTree.h"
#include "TFile.h"
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
#include "TableCollection.h"
#include "InclusiveDiffractiveCrossSectionsFromTables.h"

#include "Math/GaussLegendreIntegrator.h"
#include "Math/Integrator.h"

#include <mpi.h>

#define PR(x) cout << #x << " = " << (x) << endl;

using namespace std;
InclusiveDiffractiveCrossSections* myICS=0;
TableCollection* tc;

double mFT(double* var, double* par)
{

    // cout<<"err0 here at cross-section calclation function"<<endl;
    double beta=par[0];
    double Q2=par[1];
    double W2=par[2];
    double z=var[0];
    
    return myICS->dsigmadbetadz_T(beta, Q2, W2, z); // returns cross-section total
    // return myICS->dsigdbetadQ2dW2dz_total(beta, Q2, W2, z, transverse);
}
double mFL(double* var, double* par)
{
    double beta=par[0];
    double Q2=par[1];
    double W2=par[2];
    double z=var[0];

    return myICS->dsigmadbetadz_L(beta, Q2, W2, z);
    // return myICS->dsigdbetadQ2dW2dz_total(beta, Q2, W2, z, longitudinal);
}
double mFQQG(double* var, double* par)
{
    double beta=par[0];
    double Q2=par[1];
    double W2=par[2];
    double z=var[0];
    
    return myICS->dsigmadbetadz_QQG(beta, Q2, W2, z); //nb
    // return myICS->dsigdbetadQ2dW2dz_qqg(beta, Q2, W2, z);
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
    int    pol;      // 0=transverse or 1=longitudinal
    int    dmode;    // 0=coherent, 1=Incoherent
//    double crossSection;
};

rootSartreEvent myRootSartreEvent;
void randomlyReverseBeams(Event* );  // for UPC mode

int main(int argc, char *argv[])
{

    // MPI_Init(&argc, &argv);
    // MPI_Status status;

    // int rank, size;
    // MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    // MPI_Comm_size(MPI_COMM_WORLD, &size);

    // int MASTER = 0;
    // int BIN_TAG = 1;
    // int RESULT_TAG = 2;
    // int STOP_TAG = 3;
    
    //
    //  Check command line arguments
    //
    // if (! (argc == 2 || argc == 3) ) {
    //     cout << "Usage:  " << argv[0] << " runcard [rootfile]" << endl;
    //     return 2;
    // }
    string runcard = argv[1];
    double Q2 = stod(argv[2]);
    
    
    //To run some sanity checks on the cross sections:
    //    checksForCrossSections();
    //    exit(1);
    
    //
    //  Create the generator and initialize it.
    //  Once initialized you cannot (should not) change
    //  the settings w/o re-initialing sartre.
    //
    SartreInclusiveDiffraction sartre;
    //
    //   DGLAP Evolution can be speed up by using lookup tables
    //
    // DglapEvolution &dglap = DglapEvolution::instance();
    // dglap.generateLookupTable(1000, 1000);
    // dglap.useLookupTable(true);
    
    bool ok = sartre.init(runcard);
    if (!ok) {
        cerr << "Initialization of sartre failed." << endl;
        return 1;
    }
    
    EventGeneratorSettings* settings = sartre.runSettings();
    
    //
    //  ROOT file
    //  Use the one from command line unless not given
    //  in which case the one in the runcard is used.
    //
    string rootfile;
    if (argc == 3) {
        rootfile = argv[2];
        settings->setRootfile(argv[2]);
    }
    else
        rootfile = settings->rootfile();
    
    //
    //  Print out all Sartre settings
    //
    settings->list();
    
//    double s=4*settings->electronBeamEnergy()*settings->hadronBeamEnergy();
//    reducedCrossSections(s);
//    exit(1);
//    

    int bin  = 100;
    
    string mode; // if table is true mode is "table"; if false the mode is "theory"
    string tableBin;  // table binning 
    // double Q2 = 8.5;
    double xpom = 1e-3;
    
    double min_Rng = 0.001;
    double max_Rng = 0.9;
    TH1D FT("SF_T", "sf_T", bin, min_Rng, max_Rng);
    TH1D FL("SF_L", "sf_L", bin, min_Rng, max_Rng);
    TH1D FQQG("SF_QQG","sf_qqg", bin, min_Rng, max_Rng);
    TH1D FTot("SF_tot","sf_tot", bin, min_Rng, max_Rng);   
    // InclusiveDiffractiveCrossSections* myCS;
    
    myICS = sartre.mCrossSection;
    bool table = sartre.mUseCrossSectionTables;
    bool model = sartre.mUseCrossSectionModel;
    int a = sartre.GetA();
    // int a = myICS->mA;
    if(model)
    {
        // const int A = mA;
    DipoleModelType dipoleType = DipoleModelType::bSat;              // TODO: confirm
    DipoleModelParameterSet dipoleSet = DipoleModelParameterSet::STU; // TODO: confirm

    SurrogateGuardGridParams gridParams =
        InclusiveDiffractionCrossSectionsFromModel::gridParamsFromSettings();

    myICS = new InclusiveDiffractionCrossSectionsFromModel(a, dipoleType, dipoleSet, gridParams);
    mode = "model";
    tableBin = "model";
    }
    else if (table)
    {
        TableCollection* tc = sartre.mTableCollection;
        myICS = new InclusiveDiffractionCrossSectionsFromTables(tc);
        mode = "table";
        tableBin = "table";
    }
    else
    {
        myICS = new InclusiveDiffractiveCrossSectionsIntegrals();
        mode = "theory";
        tableBin = "";
    }
    // int a = myICS->mA;
    
    


    TF1 fT("fT", mFT, 0.001, 0.999, 3);  // Function with 1 variable and 3 pararmeters 
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

    // ROOT::Math::GaussLegendreIntegrator giT(4096); 
    // ROOT::Math::GaussLegendreIntegrator giL(4096); 
    // ROOT::Math::GaussLegendreIntegrator giQQG(4096); 

    // ROOT::Math::IntegratorOneDim giQQG(ROOT::Math::IntegrationOneDim::kADAPTIVE, 0., 1e-6, 100000);
    giQQG.SetFunction(wfQQG);


    // ROOT::Math::IntegratorOneDim giT(ROOT::Math::IntegrationOneDim::kADAPTIVE);
    // ROOT::Math::IntegratorOneDim giL(ROOT::Math::IntegrationOneDim::kADAPTIVE);
    // ROOT::Math::IntegratorOneDim giQQG(ROOT::Math::IntegrationOneDim::kADAPTIVE);

    giT.SetFunction(wfT);
    giL.SetFunction(wfL);
    giQQG.SetFunction(wfQQG);

    giT.SetAbsTolerance(0.);
    giT.SetRelTolerance(1e-8);

    giL.SetAbsTolerance(0.);
    giL.SetRelTolerance(1e-8);

    giQQG.SetAbsTolerance(0.);
    giQQG.SetRelTolerance(1e-8);

    
   
    for(int i=1; i<=bin; i++){

        
        cout<<"Filling bin# "<<i<<endl;


        
        
        double beta=FT.GetBinCenter(i);
        double x=beta*xpom;
        double W2=Kinematics::W2(Q2, x);
        
        PR(x)
        PR(W2)
        double prefactor = beta * Q2 /(4 * M_PI * M_PI * alpha_em)/1e7/hbarc2;

        double csT = 0, csL = 0 , csqqg = 0, cstot = 0;

        for (int quark = 0; quark < 4; quark++){
            myICS->setQuarkIndex(quark);
            cout<<"Calculating for quark flavor "<<quark<<endl;
        
        cout<<"factors calculated"<<endl;
        fT.SetParameter(0, beta);
        fL.SetParameter(0, beta);
        fQQG.SetParameter(0, beta);
        fT.SetParameter(2, W2);
        fL.SetParameter(2, W2);
        fQQG.SetParameter(2, W2);

        // cout<<"Parameters set "<<i<<endl;
        // double cs=giSigma.Integral(0, 1);


        cout<<"err0 before integral"<<endl;
        csT += giT.Integral(0.001, 0.999); //* fluxT; nb
        // PR(csT)
        csL += giL.Integral(0.001, 0.999); //* fluxL; nb
        // PR(csL)

        // Debug: sample integrand at several z points
// cout << "beta = " << beta << ", W2 = " << W2 << endl;
// for (double z = beta; z < 0.999; z += (0.999 - beta) / 10.0) {
//     double val = myICS->dsigdbetadQ2dW2dz_qqg(beta, Q2, W2, z);
//     cout << "beta"<< beta << "  z=" << z << "  qqg=" << val << endl;
// }    
        // double zStart = max(beta, 0.93);  // never below beta, but anchor near 1
        PR(beta)
        csqqg += giQQG.Integral(0.001, 0.999);
        PR(csqqg)
        cout<<"err0 after Integral"<<endl;

        }
        // cout<<"calclating cross section"<<endl;
        csT *= prefactor ;//* giT.Integral(0, 1);//* fluxT; 
        csL *= prefactor ;//* giL.Integral(0, 1); //* fluxL;
        csqqg *= prefactor ;//* giQQG.Integral(beta,1);//* fluxT;
        cstot = csT + csL + csqqg; 
        
        if (std::isnan(csT)) csT = 0;
        if (std::isnan(csL)) csL = 0;
        if (std::isnan(csqqg)) csqqg = 0;
        if (std::isnan(cstot)) cstot = 0;

    

        FT.SetBinContent(i, csT);
        FL.SetBinContent(i, csL);
        FQQG.SetBinContent(i,csqqg);
        FTot.SetBinContent(i,cstot);

        cout<<"Filled bin# "<<i<<endl;
    }
        
    
    TString filename = "SFvbeta_"+mode+"_A"+a+"_Q2_"+Q2+"_xpom_"+xpom+"_"+tableBin+"_EIC_"+bin;
    TFile *hfile = new TFile(filename+".root","RECREATE");
    // TFile *hfile = new TFile("null.root","RECREATE");
    FT.Write();
    FL.Write();
    FQQG.Write();
    FTot.Write();
    hfile->Write();
    delete hfile;
    // sf();
    // exit(1);
    
    cout << filename+".root" << " has been created" << endl;
 
    // MPI_Finalize();
    // return 0;

}
