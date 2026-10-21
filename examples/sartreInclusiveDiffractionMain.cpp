//==============================================================================
//  sartreInclusiveDiffractionMain.cpp
//
//  Copyright (C) 2024 Tobias Toll and Thomas Ullrich
//
//  This file is part of Sartre.
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation.
//  This program is distributed in the hope that it will be useful,
//  but without any warranty; without even the implied warranty of
//  merchantability or fitness for a particular purpose. See the
//  GNU General Public License for more details.
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
//  Author: Thomas Ullrich
//  Last update:
//  $Date: $
//  $Author: ullrich $
//==============================================================================
//
//  Example main program. Use to get started.
//
//==============================================================================
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


#include "Math/GaussLegendreIntegrator.h"
#include "Math/Integrator.h"

#include "Event.h"
#include "EicSmearFormatWriter.h"

// #define EIC_SMEAR_OUTPUT 1  // uncomment for eic-smear output

#define PR(x) cout << #x << " = " << (x) << endl;

using namespace std;
InclusiveDiffractiveCrossSections* myICS;

double uiCrossSection(double* var, double* par){
    double beta=par[0];
    double Q2=par[1];
    double W2=par[2];
    double z=var[0]; 
    // cout<< "beta: "<<beta<<" Q2: "<<Q2<<" W2: "<<W2<<" z: "<<z<<endl;
    double result = myICS->dsigdbetadQ2dW2dz_total(beta, Q2, W2, z,transverse) + myICS->dsigdbetadQ2dW2dz_qqg(beta, Q2, W2, z) + myICS->dsigdbetadQ2dW2dz_total(beta, Q2, W2, z,longitudinal);
    // double result =  myICS->dsigdbetadQ2dW2dz_qqg(beta, Q2, W2, z);//myICS->dsigdbetadQ2dW2dz_total_checked(beta, Q2, W2, z);// //myICS->dsigdbetadQ2dW2dz_qqg(beta, Q2, W2, z)
    // PR(result)
    return result; // bpn : this is not the right funciton only calls qq fock state. what abouut the qqg part ?

}

double xp_sigmar(double Q2, double beta, double xpom, double s){
    double x=beta*xpom;
    double W2=Kinematics::W2(Q2, x);
    // PR(Q2)
    // PR(xpom)
    // PR(beta)
    // PR(W2)
    // if(W2 > 240*240) cout << "W2 is too large for the model to be valid. W2 = " << W2 << endl;
    double y=Kinematics::y(Q2, x, s);
    TF1 fsigma("fsigma", uiCrossSection, 0, 1, 3);
    fsigma.SetParameter(0, beta);
    fsigma.SetParameter(1, Q2);
    fsigma.SetParameter(2, W2);

    // const double precision = 5e-4;
//  unsigned int numberofcalls = 1000000;
    // unsigned int numberofcalls = 0;//1e5;
    ROOT::Math::WrappedTF1 wfsigma(fsigma);


    ROOT::Math::GaussIntegrator giSigma; 
    // ROOT::Math::GaussLegendreIntegrator giSigma{256}; //bpn : 2048 is the number of points for the integration. can be changed to 4096 or 8192 for better precision but slower speed

    // ROOT::Math::IntegratorOneDim giSigma(ROOT::Math::IntegrationOneDim::kADAPTIVE); 

    giSigma.SetFunction(wfsigma);
    giSigma.SetAbsTolerance(0.);
    giSigma.SetRelTolerance(1e-8);
    
    // cout<<"integrating cross-section for beta = "<<beta<<", Q2 = "<<Q2<<", xpom = "<<xpom<<", s = "<<s<<endl;
    double cs=giSigma.Integral(0.001, 0.999); //nb/GeV4
    // PR(cs)
    // double csqqg = giSigmaqqg.Integral(beta, 1);
    double prefactor= 4*M_PI*alpha_em*alpha_em* xpom*xpom /(Q2*Q2*Q2);
    prefactor*=1-y+y*y/2; //GeV-6
    double sigma_r= cs/prefactor; //nb*GeV2
    sigma_r /= 1e7; //fm2*GeV2
    sigma_r /= hbarc2; //no unit;
    return xpom*sigma_r;
}
/*

//===============================================================================================================
//.    bpn modified
//===============================================================================================================
double xp_sigmar(double Q2, double beta, double xpom, double s){
    double x = beta * xpom;
    double W2 = Kinematics::W2(Q2, x);
    double y = Kinematics::y(Q2, x, s);

    TF1 fsigma("fsigma", uiCrossSection, 0, 1, 3);
    fsigma.SetParameter(0, beta);
    fsigma.SetParameter(1, Q2);
    fsigma.SetParameter(2, W2);

    ROOT::Math::WrappedTF1 wfsigma(fsigma);
    ROOT::Math::GaussIntegrator giSigma; 
    giSigma.SetFunction(wfsigma);
    giSigma.SetAbsTolerance(0.);
    giSigma.SetRelTolerance(1e-12);

    double cs = giSigma.Integral(0.001, 0.999); // nb/GeV4

    double sigma_r = cs;

    // --- Smarter multi-axis repair logic using averages ---
    const double floor = 1e-15;              
    // if (sigma_r < floor || !std::isfinite(sigma_r)) {
        const double eps_list[] = {0.02, 0.05, 0.1, 0.2}; // smallest first

        // Axes priority: Q² -> W² -> beta
        for (double eps : eps_list) {

            // --- Q² axis ---
            double Q2p = Q2*(1+eps), Q2m = Q2*(1-eps);
            double W2p = Kinematics::W2(Q2p, x), W2m = Kinematics::W2(Q2m, x);

            fsigma.SetParameter(1, Q2p); fsigma.SetParameter(2, W2p);
            double valp = giSigma.Integral(0.001,0.999);
            fsigma.SetParameter(1, Q2m); fsigma.SetParameter(2, W2m);
            double valm = giSigma.Integral(0.001,0.999);

            bool p_valid = (valp > floor && std::isfinite(valp));
            bool m_valid = (valm > floor && std::isfinite(valm));

            if (p_valid && m_valid) { sigma_r = 0.5*(valp + valm); break; }
            else if (p_valid) { sigma_r = valp; break; }
            else if (m_valid) { sigma_r = valm; break; }
            /*

            // --- W2 axis ---
            double W2p2 = W2*(1+eps), W2m2 = W2*(1-eps);
            fsigma.SetParameter(1, Q2); fsigma.SetParameter(2, W2p2);
            valp = giSigma.Integral(0.001,0.999);
            fsigma.SetParameter(2, W2m2);
            valm = giSigma.Integral(0.001,0.999);

            p_valid = (valp > floor && std::isfinite(valp));
            m_valid = (valm > floor && std::isfinite(valm));

            if (p_valid && m_valid) { sigma_r = 0.5*(valp + valm); break; }
            else if (p_valid) { sigma_r = valp; break; }
            else if (m_valid) { sigma_r = valm; break; }

            // --- beta axis ---
            double betap = beta*(1+eps), betam = beta*(1-eps);
            fsigma.SetParameter(0, betap); fsigma.SetParameter(1, Q2); fsigma.SetParameter(2, W2);
            valp = giSigma.Integral(0.001,0.999);
            fsigma.SetParameter(0, betam);
            valm = giSigma.Integral(0.001,0.999);

            p_valid = (valp > floor && std::isfinite(valp));
            m_valid = (valm > floor && std::isfinite(valm));
    

            if (p_valid && m_valid) { sigma_r = 0.5*(valp + valm); break; }
            else if (p_valid) { sigma_r = valp; break; }
            else if (m_valid) { sigma_r = valm; break; }
            
        }
    // }
    // --- End of repair logic ---

    double prefactor = 4*M_PI*alpha_em*alpha_em* xpom*xpom /(Q2*Q2*Q2);
    prefactor *= 1 - y + y*y/2; // GeV-6
    sigma_r /= prefactor; // nb*GeV2
    sigma_r /= 1e7; // fm2*GeV2
    sigma_r /= hbarc2; // no unit

    return xpom * sigma_r;
}
*/
//===============================================================================================================


void reducedCrossSections(double s, InclusiveDiffractiveCrossSections* myICS)
{
    // TableCollection *tc;
    // myICS = new InclusiveDiffractiveCrossSectionsIntegrals();
    // myICS = new InclusiveDiffractionCrossSectionsFromTables(tc);
    // cout << "QQG Contribution" << endl; 
    double Q2val[80]={2.5, 2.5, 5.1, 8.8, 2.5, 2.5, 2.5, 5.1, 5.1, 8.8, 8.8, 15.3, 26.5, 46, 2.5, 2.5, 2.5, 2.5, 5.1, 5.1, 5.1, 8.8, 8.8, 8.8, 15.3, 15.3, 26.5, 26.5, 46, 46, 2.5, 2.5, 2.5, 2.5, 5.1, 5.1, 5.1, 5.1, 8.8, 8.8, 8.8, 8.8, 15.3, 15.3, 15.3, 26.5, 26.5, 26.5, 46, 46, 46, 80, 80, 200, 2.5, 2.5, 2.5, 2.5, 5.1, 5.1, 5.1, 5.1, 8.8, 8.8, 8.8, 8.8, 15.3, 15.3, 15.3, 15.3, 26.5, 26.5, 46, 46, 46, 46, 80, 80, 200, 200};
    double betaval[80]={0.1780, 0.5620, 0.5620, 0.5620, 0.0562, 0.1780, 0.5620, 0.1780, 0.5620, 0.1780, 0.5620, 0.5620, 0.5620, 0.8160, 0.0178, 0.0562, 0.1780, 0.5620, 0.0562, 0.1780, 0.5620, 0.0562, 0.1780, 0.5620, 0.1780, 0.5620, 0.1780, 0.5620, 0.5620, 0.8160, 0.0056, 0.0178, 0.0562, 0.1780, 0.0178, 0.0562, 0.1780, 0.5620, 0.0178, 0.0562, 0.1780, 0.5620, 0.0562, 0.1780, 0.5620, 0.0562, 0.1780, 0.5620, 0.1780, 0.5620, 0.8160, 0.1780, 0.5620, 0.5620, 0.0056, 0.0178, 0.0562, 0.1780, 0.0056, 0.0178, 0.0562, 0.1780, 0.0178, 0.0562, 0.1780, 0.5620, 0.0178, 0.0562, 0.1780, 0.5620, 0.0562, 0.1780, 0.0562, 0.1780, 0.5620, 0.8160, 0.1780, 0.5620, 0.1780, 0.5620};
    double xpomval[80]={0.0003, 0.0003, 0.0003, 0.0003, 0.0009, 0.0009, 0.0009, 0.0009, 0.0009, 0.0009, 0.0009, 0.0009, 0.0009, 0.0009, 0.0025, 0.0025, 0.0025, 0.0025, 0.0025, 0.0025, 0.0025, 0.0025, 0.0025, 0.0025, 0.0025, 0.0025, 0.0025, 0.0025, 0.0025, 0.0025, 0.0085, 0.0085, 0.0085, 0.0085, 0.0085, 0.0085, 0.0085, 0.0085, 0.0085, 0.0085, 0.0085, 0.0085, 0.0085, 0.0085, 0.0085, 0.0085, 0.0085, 0.0085, 0.0085, 0.0085, 0.0085, 0.0085, 0.0085, 0.0085, 0.0160, 0.0160, 0.0160, 0.0160, 0.0160, 0.0160, 0.0160, 0.0160, 0.0160, 0.0160, 0.0160, 0.0160, 0.0160, 0.0160, 0.0160, 0.0160, 0.0160, 0.0160, 0.0160, 0.0160, 0.0160, 0.0160, 0.0160, 0.0160, 0.0160, 0.0160};
    double xsigma_rval[80]={0.0156, 0.0214, 0.0275, 0.0250, 0.0114, 0.0102, 0.0172, 0.0121, 0.0187, 0.0177, 0.0207, 0.0180, 0.0241, 0.0145, 0.0099, 0.0074, 0.0068, 0.0110, 0.0107, 0.0118, 0.0153, 0.0125, 0.0129, 0.0166, 0.0136, 0.0173, 0.0138, 0.0189, 0.0196, 0.0131, 0.0101, 0.0076, 0.0064, 0.0074, 0.0120, 0.0088, 0.0095, 0.0137, 0.0128, 0.0106, 0.0104, 0.0142, 0.0134, 0.0111, 0.0162, 0.0157, 0.0126, 0.0140, 0.0121, 0.0135, 0.0110, 0.0206, 0.0116, 0.0109, 0.0093, 0.0073, 0.0068, 0.0116, 0.0135, 0.0111, 0.0088, 0.0075, 0.0124, 0.0108, 0.0090, 0.0102, 0.0176, 0.0122, 0.0098, 0.0151, 0.0150, 0.0113, 0.0163, 0.0133, 0.0124, 0.0092, 0.0133, 0.0090, 0.0145, 0.0093};
    double deltaval[80]={12, 19, 16, 13, 17, 8.8, 31, 18, 12, 9.7, 10, 11, 25, 22, 18, 12, 12, 36, 6.8, 7.4, 8.7, 7.1, 6.1, 10, 6.2, 7.2, 7.6, 12, 8.6, 20, 25, 12, 13, 15, 10, 6.7, 6.7, 27, 7.8, 6.3, 5.9, 13, 9.7, 6.2, 6.8, 12, 7.5, 7.6, 8.9, 8.5, 19, 16, 13, 21, 15, 13, 16, 20, 7.4, 6.5, 6.6, 19, 6.0, 6.4, 7.1, 22, 7.4, 6.8, 6.8, 22, 7.2, 7.9, 11, 8.1, 8.9, 28, 14, 16, 21, 24};
    
    double beta=0.0056;//betaval[i];
    double xpom=0.0085;//xpomval[i];
    double Q2=2.5;
    cout<<"sartre_0_0=["<<xp_sigmar(Q2, beta, xpom, s)<<"]"<<endl;
    
    //    xp_0_0085_beta_0_0178=[[2.5,5.1,8.8,],[0.0076,0.012,0.0128],

    beta=0.0178;
    xpom=0.0085;
    double Q2_1[3]={2.5, 5.1, 8.8};
    cout<<"sartre_0_1=[";
    for(int i=0; i<3; i++){
        cout<<xp_sigmar(Q2_1[i], beta, xpom, s);
        if(i<2) cout<<", ";
    }
    cout<<"]"<<endl;
    
    //    xp_0_0085_beta_0_0562=[[2.5,5.1,8.8,15.3,26.5],
    beta=0.0562;
    xpom=0.0085;
    double Q2_2[5]={2.5,5.1,8.8,15.3,26.5};
    cout<<"sartre_0_2=[";
    for(int i=0; i<5; i++){
        cout<<xp_sigmar(Q2_2[i], beta, xpom, s);
        if(i<4) cout<<", ";
    }
    cout<<"]"<<endl;
    
    //    xp_0_0085_beta_0_178=[[2.5,5.1,8.8,15.3,26.5,46,80]
    xpom=0.0085;
    beta=0.178;
    double Q2_3[7]={2.5,5.1,8.8,15.3,26.5,46,80};
    cout<<"sartre_0_3=[";
    for(int i=0; i<7; i++){
        cout<<xp_sigmar(Q2_3[i], beta, xpom, s);
        if(i<6) cout<<", ";
    }
    cout<<"]"<<endl;
    
    //    xp_0_0085_beta_0_562=[[5.1,8.8,15.3,26.5,46.0,80.0,200.0]
    xpom=0.0085;
    beta=.562;
    double Q2_4[7]={5.1,8.8,15.3,26.5,46.0,80.0,200.0};
    cout<<"sartre_0_4=[";
    for(int i=0; i<7; i++){
        cout<<xp_sigmar(Q2_4[i], beta, xpom, s);
        if(i<6) cout<<", ";
    }
    cout<<"]"<<endl;
    
    //    xp_0_0085_beta_0_816=[[46.0]
    xpom=0.0085;
    beta=0.816;
    Q2=46.;
    cout<<"sartre_0_5=["<<xp_sigmar(Q2, beta, xpom, s)<<"]"<<endl;
    
    //    xp_0_0025_beta_0_0178=[[2.5],
    xpom=0.0025;
    beta=0.0178;
    Q2=2.5;
    cout<<"sartre_1_1=["<<xp_sigmar(Q2, beta, xpom, s)<<"]"<<endl;
    
    //    xp_0_0025_beta_0_0562=[[2.5,5.1,8.8]
    xpom=0.0025;
    beta=0.0562;
    double Q2_5[3]={2.5,5.1,8.8};
    cout<<"sartre_1_2=[";
    for(int i=0; i<3; i++){
        cout<<xp_sigmar(Q2_5[i], beta, xpom, s);
        if(i<2) cout<<", ";
    }
    cout<<"]"<<endl;
    
    //    xp_0_0025_beta_0_178=[[2.5,5.1,8.8,15.3,26.5]
    xpom=0.0025;
    beta=0.178;
    double Q2_6[5]={2.5,5.1,8.8,15.3,26.5};
    cout<<"sartre_1_3=[";
    for(int i=0; i<5; i++){
        cout<<xp_sigmar(Q2_6[i], beta, xpom, s);
        if(i<4) cout<<", ";
    }
    cout<<"]"<<endl;
    
    //    xp_0_0025_beta_0_562=[[2.5,5.1,8.8,15.3,26.5,46.0]
    xpom=0.0025;
    beta=0.562;
    double Q2_7[6]={2.5,5.1,8.8,15.3,26.5,46.0};
    cout<<"sartre_1_4=[";
    for(int i=0; i<6; i++){
        cout<<xp_sigmar(Q2_7[i], beta, xpom, s);
        if(i<5) cout<<", ";
    }
    cout<<"]"<<endl;
    
    //    xp_0_0025_beta_0_816=[[46],
    xpom=0.0025;
    beta=0.816;
    Q2=46.;
    cout<<"sartre_1_5=["<<xp_sigmar(Q2, beta, xpom, s)<<"]"<<endl;
    
    //    xp_0_0009_beta_0_0562=[[2.5]
    xpom=0.0009;
    beta=0.0562;
    Q2=2.5;
    cout<<"sartre_2_2=["<<xp_sigmar(Q2, beta, xpom, s)<<"]"<<endl;
    
    //    xp_0_0009_beta_0_178=[[2.5,5.1,8.8]
    xpom=0.0009;
    beta=0.178;
    double Q2_8[3]={2.5,5.1,8.8};
    cout<<"sartre_2_3=[";
    for(int i=0; i<3; i++){
        cout<<xp_sigmar(Q2_8[i], beta, xpom, s);
        if(i<2) cout<<", ";
    }
    cout<<"]"<<endl;
    
    //    xp_0_0009_beta_0_562=[[2.5,5.1,8.8,15.3,26.5]
    xpom=0.0009;
    beta=0.562;
    double Q2_9[5]={2.5,5.1,8.8,15.3,26.5};
    cout<<"sartre_2_4=[";
    for(int i=0; i<5; i++){
        cout<<xp_sigmar(Q2_9[i], beta, xpom, s);
        if(i<4) cout<<", ";
    }
    cout<<"]"<<endl;
    
    //    xp_0_0009_beta_0_816=[[46.0]
    xpom=0.0009;
    beta=0.816;
    Q2=46.;
    cout<<"sartre_2_5=["<<xp_sigmar(Q2, beta, xpom, s)<<"]"<<endl;
    
    //    xp_0_0003_beta_0_178=[[2.5],[0.0156]
    xpom=0.0003;
    beta=0.178;
    Q2=2.5;
    cout<<"sartre_3_3=["<<xp_sigmar(Q2, beta, xpom, s)<<"]"<<endl;

//    xp_0_0003_beta_0_562=[[2.5,5.1,8.8]
    xpom=0.0003;
    beta=0.562;
    double Q2_10[3]={2.5,5.1,8.8};
    cout<<"sartre_3_4=[";
    for(int i=0; i<3; i++){
        cout<<xp_sigmar(Q2_10[i], beta, xpom, s);
        if(i<2) cout<<", ";
    }
    cout<<"]"<<endl;
    
}
void checksForCrossSections(){
    cout<<"#######################################"<<endl;
    cout<<"###      Doing some sanity checks   ###"<<endl;
    cout<<"#######################################"<<endl;
    TH1D cs_beta_T("cs_beta_T", "cs_beta_T", 100, 0, 1);
    TH1D cs_beta_L("cs_beta_L", "cs_beta_L", 100, 0, 1);
    InclusiveDiffractiveCrossSections myCS;
    double Q2=5;
    double xpom=1e-3;
    double z=0.1;
    cout<<"Filling the cross section histograms binned in beta:"<<endl;
    for(int i=1; i<=100; i++){
        cout<<"Filling bin# "<<i<<endl;
        double beta=cs_beta_T.GetBinCenter(i);
        double MX2=(1-beta)/beta*Q2;
        double W2=Kinematics::W2(Q2, xpom, MX2);
        myCS.setQuarkIndex(0);
        double csT=myCS.dsigmadbetadz_T(beta, Q2, W2, z);
        double csL=myCS.dsigmadbetadz_L(beta, Q2, W2, z);
        cs_beta_T.SetBinContent(i, csT);
        cs_beta_L.SetBinContent(i, csL);
    }
    TFile *hfile = new TFile("CStestsBeta.root","RECREATE");
    cs_beta_T.Write();
    cs_beta_L.Write();
    hfile->Write();
    delete hfile;
}
// ============================================================
// main.cpp
// ============================================================

//
//  Particle record written to the "particles" branch.
//  Carries PDG id, status, and parent/daughter indices (matching
//  Particle::index in event->particles) so the full decay/shower
//  graph -- beams, scattered e/p, quark/antiquark/gluon, Pythia
//  secondaries, nuclear breakup -- can be walked in analysis.
//
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
    int    iEvent;
    int    pol;      // 0=transverse or 1=longitudinal
    int    dmode;    // 0=coherent, 1=Incoherent
//    double crossSection;
};

rootSartreEvent myRootSartreEvent;
void randomlyReverseBeams(Event* );  // for UPC mode

int main(int argc, char *argv[])
{
    //
    //  Check command line arguments
    //
    if (! (argc == 2 || argc == 3) ) {
        cout << "Usage:  " << argv[0] << " runcard [rootfile]" << endl;
        return 2;
    }
    string runcard = argv[1];

    //
    //  Create the generator and initialize it.
    //  Once initialized you cannot (should not) change
    //  the settings w/o re-initialing sartre.
    //
    SartreInclusiveDiffraction sartre;

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

    TFile *hfile = 0;
    if (rootfile.size()) {
        hfile  = new TFile(rootfile.c_str(),"RECREATE");
        cout << "ROOT file is '" <<  rootfile.c_str() << "'." << endl;
    }

    myICS = sartre.mCrossSection;
    double s=4*settings->electronBeamEnergy()*settings->hadronBeamEnergy();
    PR(settings->electronBeamEnergy())
    PR(settings->hadronBeamEnergy())
    PR(s)
    reducedCrossSections(s, myICS);

    #if defined(EIC_SMEAR_OUTPUT)
    string eicSmearFilename;
    eicSmearFilename = rootfile.substr(0, rootfile.find(".root"));
    eicSmearFilename += ".out";
    EicSmearFormatWriter eicSmearWriter;
    eicSmearWriter.open(eicSmearFilename, settings->enableNuclearBreakup());
    cout << "eic-smear compatible output file is '" <<  eicSmearFilename.c_str() << "'." << endl;
    #endif

    //
    //  Setup ROOT tree
    //
    TLorentzVector *eIn = new TLorentzVector;
    TLorentzVector *pIn = new TLorentzVector;
    TLorentzVector *eOut = new TLorentzVector;
    TLorentzVector *pOut = new TLorentzVector;
    TLorentzVector *gamma = new TLorentzVector;
    TLorentzVector *quark = new TLorentzVector;
    TLorentzVector *antiquark = new TLorentzVector;
    TLorentzVector *gluon = new TLorentzVector;

    TTree tree("tree","sartre");
    tree.Branch("event", &myRootSartreEvent.t,
                "t/D:beta/D:Q2/D:x/D:s/D:y/D:W/D:z/D:MX/D:xpom/D:iEvent/I:pol/I:dmode/I");
    tree.Branch("eIn",  "TLorentzVector", &eIn, 32000, 0);
    tree.Branch("pIn",  "TLorentzVector", &pIn, 32000, 0);
    tree.Branch("eOut", "TLorentzVector", &eOut, 32000, 0);
    tree.Branch("pOut", "TLorentzVector", &pOut, 32000, 0);
    tree.Branch("gamma","TLorentzVector", &gamma, 32000, 0);
    tree.Branch("quark", "TLorentzVector", &quark, 32000, 0);
    tree.Branch("antiquark", "TLorentzVector", &antiquark, 32000, 0);
    tree.Branch("gluon", "TLorentzVector", &gluon, 32000, 0);

    //
    //  To hold the Pythia generated particles (restored)
    //
    TClonesArray pythia("TLorentzVector");
    tree.Branch("pythia", &pythia);

    //
    //  Full particle record for the WHOLE event -- beams, scattered
    //  e/p, quark/antiquark/gluon, Pythia secondaries, and nuclear
    //  breakup remnants -- as plain vectors ROOT already knows how
    //  to write (no custom class, no dictionary needed).
    //  All arrays below are index-aligned: entry i in every branch
    //  describes the same particle, and particleParents[i] /
    //  particleDaughters[i] hold the indices (into these same
    //  arrays) of that particle's parents/daughters.
    //
    std::vector<int>                particleIndex;
    std::vector<int>                particlePdgId;
    std::vector<int>                particleStatus;
    std::vector<std::vector<int>>   particleParents;
    std::vector<std::vector<int>>   particleDaughters;
    TClonesArray particleP("TLorentzVector");

    tree.Branch("particleIndex", &particleIndex);
    tree.Branch("particlePdgId", &particlePdgId);
    tree.Branch("particleStatus", &particleStatus);
    tree.Branch("particleParents", &particleParents);
    tree.Branch("particleDaughters", &particleDaughters);
    tree.Branch("particleP", &particleP);

    //
    //  For the nuclear breakup particles
    //
    TClonesArray protons("TLorentzVector");
    TClonesArray neutrons("TLorentzVector");
    TClonesArray remnants("TLorentzVector");
    if(settings->enableNuclearBreakup()){
        tree.Branch("protons", &protons);
        tree.Branch("neutrons", &neutrons);
        tree.Branch("nuclearRemnants", &remnants);
    }

    int nPrint;
    if (settings->timesToShow())
        nPrint = settings->numberOfEvents()/settings->timesToShow();
    else
        nPrint = 0;

    unsigned long maxEvents = settings->numberOfEvents();

    cout << "Generating " << maxEvents << " events." << endl << endl;

    //
    //  Event generation
    //
    for (unsigned long iEvent = 0; iEvent < maxEvents; iEvent++) {
        //
        //  Generate one event
        //
        Event *event = sartre.generateEvent();
        if (nPrint && (iEvent+1)%nPrint == 0 && iEvent != 0) {
            cout << "processed " << iEvent+1 << " events" << endl;
        }

        //
        //  If Sartre is run in UPC mode, half of the events needs to be
        //  rotated around and axis perpendicular to z:
        //
        if(settings->UPC() and settings->A()==settings->UPCA()){
            randomlyReverseBeams(event);
        }

        //
        //  Print out (here only for the first few events)
        //
        if (iEvent < 10) event->list();

        //
        //  Fill ROOT tree
        //
        myRootSartreEvent.iEvent = event->eventNumber;
        myRootSartreEvent.t = event->t;
        myRootSartreEvent.beta = event->beta;
        myRootSartreEvent.Q2 = event->Q2;
        myRootSartreEvent.x = event->x;
        myRootSartreEvent.s = event->s;
        myRootSartreEvent.y = event->y;
        myRootSartreEvent.W = event->W;
        myRootSartreEvent.z = event->z;
        myRootSartreEvent.MX = event->MX;
        myRootSartreEvent.xpom = event->xpom;
        myRootSartreEvent.pol = event->polarization == transverse ? 0 : 1;
        myRootSartreEvent.dmode = event->diffractiveMode == coherent ? 0 : 1;
        eIn      = &event->particles[0].p;
        pIn      = &event->particles[1].p;
        eOut     = &event->particles[2].p;
        pOut     = &event->particles[4].p;
        gamma    = &event->particles[3].p;
        quark    = &event->particles[5].p;
        antiquark= &event->particles[6].p;
        if(sartre.isQQ == false)
            gluon    = &event->particles[7].p;

        else
            gluon    = 0;
        
        //
        //   Now what comes out of the pythia afterburner (only stable)
        //
        pythia.Clear();
        unsigned int kParticle;
        for (kParticle=7; kParticle < event->particles.size() && kParticle < 7+event->numberOfPythiaParticlesStored; kParticle++) {
            if(event->particles[kParticle].status == 1) {  // Final-state particle
                const Particle& particle = event->particles[kParticle];
                new(pythia[pythia.GetEntries()]) TLorentzVector(particle.p);
            }  // if
        }

        //
        //  Full particle graph for this event: every particle Sartre
        //  and Pythia produced, with lineage.
        //
        particleIndex.clear();
        particlePdgId.clear();
        particleStatus.clear();
        particleParents.clear();
        particleDaughters.clear();
        particleP.Clear();
        for (const auto& particle : event->particles) {
            particleIndex.push_back(particle.index);
            particlePdgId.push_back(particle.pdgId);
            particleStatus.push_back(particle.status);
            particleParents.push_back(particle.parents);
            particleDaughters.push_back(particle.daughters);
            new(particleP[particleP.GetEntries()]) TLorentzVector(particle.p);
        }

        //
        //  If the event is incoherent, and nuclear breakup is enabled,
        //  fill the remnants to the tree. Breakup particles start
        //  right after the fixed Sartre particles (0..6) plus however
        //  many Pythia secondaries were stored.
        //
        if(settings->enableNuclearBreakup() and event->diffractiveMode == incoherent){
            protons.Clear();
            neutrons.Clear();
            remnants.Clear();

            unsigned int breakupStart = 7 + event->numberOfPythiaParticlesStored;
            for(unsigned int iParticle=breakupStart; iParticle < event->particles.size(); iParticle++){
                const Particle& particle = event->particles[iParticle];
                if (particle.status == 1) {  // Final-state particle
                    switch (abs(particle.pdgId)) {
                        case 2212:  // (Anti-)proton
                            new(protons[protons.GetEntries()]) TLorentzVector(particle.p);
                            break;
                        case 2112:  // (Anti-)neutron
                            new(neutrons[neutrons.GetEntries()]) TLorentzVector(particle.p);
                            break;
                        default:  // Any other remnant
                            new(remnants[remnants.GetEntries()]) TLorentzVector(particle.p);
                            break;
                    }  // switch
                }  // if
            }  // for
        }  // if

        tree.Fill();
        #if defined(EIC_SMEAR_OUTPUT)
        eicSmearWriter.writeInclusiveDiffractionEvent(event);
        #endif
    }
    cout << "All events processed\n" << endl;

    //
    //  That's it, finish up
    //
    cout<<"Calculating the total cross section on given interval."<<endl;
    cout<<"This may take a while, use your radical freedom to do something else..."<<endl;
    double totalCS=sartre.totalCrossSection();
    TH1D* histoForCSandNumberOfEvents = new TH1D("histoForCSandNumberOfEvents", "Cross-section and Number of Events", 2, 0., 1.);
    histoForCSandNumberOfEvents->SetBinContent(1, totalCS);
    histoForCSandNumberOfEvents->SetBinContent(2, maxEvents);

    double runTime = sartre.runTime();
    hfile->Write();
    cout << rootfile.c_str() << " written." << endl;

    #if defined(EIC_SMEAR_OUTPUT)
    eicSmearWriter.close();
    cout << "File '" << eicSmearWriter.filename() << "' written." << endl;
    #endif
    cout << "Total cross-section: " << totalCS << " nb" << endl;
    sartre.listStatus();
    cout << "CPU Time/event: " << runTime/maxEvents << " seconds/event" << endl;
    return 0;
}

// UPC only
void randomlyReverseBeams(Event* myEvent) {

    TRandom3 *random = EventGeneratorSettings::randomGenerator();

    if(random->Uniform(1) > 0.5){
        for(unsigned int i=0; i<myEvent->particles.size(); i++)
            myEvent->particles.at(i).p.RotateX(M_PI);
    }
}