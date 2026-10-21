// // //==============================================================================
// // //  InclusiveDiffractiveCrossSection.cpp
// // //
// // //  Copyright (C) 2024 Tobias Toll and Thomas Ullrich
// // //
// // //  This file is part of Sartre.
// // //
// // //  This program is free software: you can redistribute it and/or modify
// // //  it under the terms of the GNU General Public License as published by
// // //  the Free Software Foundation.
// // //  This program is distributed in the hope that it will be useful,
// // //  but without any warranty; without even the implied warranty of
// // //  merchantability or fitness for a particular purpose. See the
// // //  GNU General Public License for more details.
// // //  You should have received a copy of the GNU General Public License
// // //  along with this program.  If not, see <http://www.gnu.org/licenses/>.
// // //
// // //  Author: Tobias Toll
// // //  Last update:
// // //  $Date: 2024-06-03 11:27:05 -0400 (Mon, 03 Jun 2024) $
// // //  $Author: ullrich $
// // //==============================================================================
// // #include "Math/SpecFunc.h"
// // #include "TFile.h"
// // #include <iostream>
// // #include <cmath>
// // #include <algorithm>
// // #include <algorithm>
// // #include "Constants.h"
// // #include "Nucleus.h"
// // #include "DipoleModel.h"
// // #include "AlphaStrong.h"
// // #include "Math/IntegratorMultiDim.h"
// // #include "Math/Functor.h"
// // #include "TMath.h"
// // #include "WaveOverlap.h"
// // #include "Kinematics.h"
// // //#include "TableGeneratorSettings.h"
// // #include "Settings.h"
// // #include "Enumerations.h"
// // #include "TF1.h"
// // #include "TF1.h"
// // #include "TH1F.h"
// // #include "cuba.h"
// // #include "InclusiveDiffractiveCrossSections.h"
// // #include "DglapEvolution.h"
// // #include "Math/WrappedTF1.h"
// // #include "Math/GaussIntegrator.h"
// // #include "DipoleModelParameters.h"

// // #include "cuba.h"


// // #define PR(x) cout << #x << " = " << (x) << endl;

// // InclusiveDiffractiveCrossSections::InclusiveDiffractiveCrossSections(){
// //     mSettings = EventGeneratorSettings::instance();
// //     mRandom = mSettings->randomGenerator();
// //     mS = Kinematics::s(mSettings->eBeam(), mSettings->hBeam());
// //     // PR(mS)
// //     mPhotonFlux.setS(mS);
// //     mCheckKinematics = true;
// //     mCrossSectionRatioLT = 0;
    
// //     mQ2=0;
    
// // //    TableGeneratorSettings* settings = TableGeneratorSettings::instance();
// //     // if(mDipoleModel) delete mDipoleModel;
    
// //     mDipoleModelParameterSet = mSettings->dipoleModelParameterSet();
// //     DipoleModelType model=mSettings->dipoleModelType();
// //     if (model==bSat) {
// //         mDipoleModel = new DipoleModel_bSat(mSettings);
// //     }
// //     else if(model==bNonSat){
// //         mDipoleModel = new DipoleModel_bNonSat(mSettings);
// //     }
// //     else {
// //         cout << "Integrals::init(): Error, model not implemented: "<< model << endl;
// //         exit(1);
// //     }
// //     mA=mSettings->A();

// // }

// // InclusiveDiffractiveCrossSections::~InclusiveDiffractiveCrossSections(){}

// // double* InclusiveDiffractiveCrossSections::dsigdbetadQ2dWdz_T_total(){return mDsigdbetadQ2dWdz_T_total;}
// // double* InclusiveDiffractiveCrossSections::dsigdbetadQ2dWdz_L_total(){return mDsigdbetadQ2dWdz_L_total;}
// // double* InclusiveDiffractiveCrossSections::dsigdbetadQ2dWdz_T_qqg(){ return mDsigdbetadQ2dWdz_T_qqg;};

// // //These three functions are unique for the sub-classes:
// // double InclusiveDiffractiveCrossSections::dsigmadbetadz_T(double, double, double, double){ return 0; }
// // double InclusiveDiffractiveCrossSections::dsigmadbetadz_L(double, double, double, double){ return 0; }
// // double InclusiveDiffractiveCrossSections::dsigmadbetadz_QQG(double, double, double, double){return 0;}
// // void InclusiveDiffractiveCrossSections::setTableCollection(TableCollection*){};



// // void InclusiveDiffractiveCrossSections::setCheckKinematics(bool val) {mCheckKinematics = val;}

// // void InclusiveDiffractiveCrossSections::setFockState(FockState val){
// //     mFockState=val;
// // }

// // FockState InclusiveDiffractiveCrossSections::getFockState(){
// //     return mFockState;
// // }

// // double InclusiveDiffractiveCrossSections::operator()(double MX2, double Q2, double W2, double z){
// //     return dsigdMX2dQ2dW2dz_total_checked(MX2, Q2, W2, z);
// // }

// // double InclusiveDiffractiveCrossSections::operator()(const double* array){
// //     double result=0;
// //     if(mFockState==QQ or mFockState==ALL){
// //         result+=dsigdbetadQ2dW2dz_total_checked(array[0], array[1], array[2], array[3]);
// //     }
// //     else if(mFockState==QQG or
// //             mFockState==ALL){
// //         result+=dsigdbetadQ2dW2dz_total_qqg_checked(array[0], array[1], array[2], array[3]);
// //     }
// //     else{
// //         cout<<"InclusiveDiffractiveCrossSectionsIntegrals::operator(): Fockstate is invalid, stopping"<<endl;
// //         exit(0);
// //     }
// //     return result;
// // }

// // double InclusiveDiffractiveCrossSections::unuranPDF(const double* array){
// //     //
// //     // array is: beta, log(Q2), W2, z
// //     //
// //     double result = 0;
// //     double Q2 = exp(array[1]);
// //     result = dsigdbetadQ2dW2dz_total_checked(array[0], Q2, array[2], array[3]);
// //     result *= Q2;   // Jacobian
// //     return log(result);
// // }

// // double InclusiveDiffractiveCrossSections::dsigdMX2dQ2dW2dz_total_checked(double MX2, double Q2, double W2, double z){
// //     double beta=Q2/(Q2+MX2);
// //     double jacobian=beta*beta/Q2;
// //     double result= dsigdbetadQ2dW2dz_total_checked(beta, Q2, W2, z);
// //     return jacobian*result;
// // }

// // double InclusiveDiffractiveCrossSections::dsigdbetadQ2dW2dz_total_checked(double beta, double Q2, double W2, double z){
// //     double result = 0;
// //     //
// //     //  Check if in kinematically allowed region
// //     double MX2=Kinematics::MX2(Q2, beta, 0);
// //     if (mCheckKinematics && !Kinematics::valid(mS, beta, Q2, W2, z, sqrt(MX2), false, (mSettings->verboseLevel() > 2) )) {
// //         if (mSettings->verboseLevel() > 2)
// //             cout << "InclusiveDiffractiveCrossSectionsIntegrals::dsigdbetadQ2dW2dz_total_checked(): warning, beta=" << beta << ", Q2=" << Q2 << ", W=" << sqrt(W2) << ", z="<< z
// //                 << " is outside of kinematically allowed region. Return 0." << endl;
// //         return 0;
// //     }
    
// //     double csT = dsigdbetadQ2dW2dz_total(beta, Q2, W2, z, transverse);
// //     double csL = dsigdbetadQ2dW2dz_total(beta, Q2, W2, z, longitudinal);
// //     result = csT + csL;
// //     mCrossSectionRatioLT=csL/csT;
// //     //
// //     //  Polarization
// //     //
// //     if (mRandom->Uniform(result) <= csT)
// //         mPolarization = transverse;
// //     else
// //         mPolarization = longitudinal;
// //     //
// //     //  Quark Species
// //     //
// //     //
    
// //     if(mPolarization == transverse){
// //         bool isChosen=false;
// //         double cs_rejected=0;
// //         for(int i=0; i<4; i++){
// //             double R=mRandom->Uniform(csT-cs_rejected);
// //             double csTi=mDsigdbetadQ2dWdz_T_total[i];
// //             if(R <= csTi){
// //                 mQuarkSpecies=i;
// //                 isChosen=true;
// //                 break;
// //             }
// //             cs_rejected+=mDsigdbetadQ2dWdz_T_total[i];
// //         }
// //         if(!isChosen) mQuarkSpecies=4;
// //     }
// //     else{
// //         bool isChosen=false;
// //         double cs_rejected=0;
// //         for(int i=0; i<4; i++){
// //             double R=mRandom->Uniform(csL-cs_rejected);
// //             double csLi=mDsigdbetadQ2dWdz_L_total[i];
// //             if(R <= csLi){
// //                 mQuarkSpecies=i;
// //                 isChosen=true;
// //                 break;
// //             }
// //             cs_rejected+=mDsigdbetadQ2dWdz_L_total[i];
// //         }
// //         if(!isChosen) mQuarkSpecies=4;
// //     }
    
// //     // Print-out at high verbose levels
// //     //
// //     if (mSettings->verboseLevel() > 10) {     // Spinal Tap ;-)
// //         cout << "InclusiveDiffractiveCrossSectionsIntegrals::dsigdbetadQ2dW2dz_total_checked(): " << result;
// //         cout << " at beta=" << beta << ", Q2=" << Q2 << ", W=" << sqrt(W2) << ", z=" << z;
// //         cout << " (" << (mPolarization == transverse ? "transverse" : "longitudinal");
// //         cout << ')' << endl;
// //     }
// //     //
// //     // Check validity of return value
// //     //
// //     if (std::isnan(result)) {
// //         cout << "InclusiveDiffractiveCrossSectionsIntegrals::dsigdbetadQ2dW2dz_total_checked(): Error, return value = NaN at" << endl;
// //         cout << "                                            beta=" << beta << ", Q2=" << Q2 << ", W=" << sqrt(W2) << ", z=" << z << endl;
// //         result = 0;
// //     }
// //     if (std::isinf(result)) {
// //         cout << "InclusiveDiffractiveCrossSectionsIntegrals::dsigdbetadQ2dW2dz_total_checked(): Error, return value = inf at" << endl;
// //         cout << "                                            beta=" << beta << ", Q2=" << Q2 << ", W=" << sqrt(W2) << ", z=" << z << endl;
// //         result = 0;
// //     }
// //     if (result < 0) {
// //         cout << "InclusiveDiffractiveCrossSectionsIntegrals::dsigdbetadQ2dW2dz_total_checked(): Error, negative cross-section at" << endl;
// //         cout << "                                            beta=" << beta << ", Q2=" << Q2 << ", W=" << sqrt(W2) << ", z=" << endl;
// //         result = 0;
// //     }
// //     mTotalCS=result;
// //     return result; //nb/GeV4
// // }

// // double InclusiveDiffractiveCrossSections::dsigdbetadQ2dW2dz_total(double beta, double Q2, double W2, double z, GammaPolarization pol) {
// //     double result = 0;
// //     for(int i=0; i<4; i++){
// //     // int i = 1;
// //         setQuarkIndex(i);
// //         double tmpresult = dsigdbetadz_total(beta, Q2, W2, z, pol); //nb
// //         if (mSettings->applyPhotonFlux()) tmpresult *= mPhotonFlux(Q2,W2,pol); //nb/GeV4
// //         if(pol == transverse)
// //             mDsigdbetadQ2dWdz_T_total[i]=tmpresult;
// //         if(pol == longitudinal)
// //             mDsigdbetadQ2dWdz_L_total[i]=tmpresult;
// //         result+=tmpresult;
// //     }
// //     return result; //nb/GeV4
// // }

// // double InclusiveDiffractiveCrossSections::dsigdbetadz_total(double beta, double Q2, double W2, double z, GammaPolarization pol) {
// //     double result = 0;
// //     if(pol==transverse){
// //         result = dsigmadbetadz_T(beta, Q2, W2, z);
// //     }
// //     else if(pol==longitudinal){
// //         result = dsigmadbetadz_L(beta, Q2, W2, z);
// //     }
// //     return result; //nb
// // }


// // double InclusiveDiffractiveCrossSections::crossSectionOfLastCall(){
// //     return mTotalCS;
// // }

// // GammaPolarization InclusiveDiffractiveCrossSections::polarizationOfLastCall(){
// //     return mPolarization;
// // }

// // unsigned int InclusiveDiffractiveCrossSections::quarkSpeciesOfLastCall(){
// //     return mQuarkSpecies;
// // }

// // double InclusiveDiffractiveCrossSections::quarkMassOfLastCall(){
// //     return Settings::quarkMass(mQuarkSpecies);
// // }

// // void InclusiveDiffractiveCrossSections::setQuarkIndex(unsigned int val){mQuarkIndex=val;}

// // DipoleModel* InclusiveDiffractiveCrossSections::dipoleModel(){
// //     return mDipoleModel;
// // }

// // double InclusiveDiffractiveCrossSections::crossSectionRatioLTOfLastCall() const {return mCrossSectionRatioLT;}


// // //===================================================
// // //           QQG Calculations
// // //===================================================
// // double InclusiveDiffractiveCrossSections::unuranPDF_qqg(const double* array){
// //     //
// //     // array is: beta, log(Q2), W2, z
// //     //
// //     double result = 0;
// //     double Q2 = exp(array[1]);

// //     double z=array[3];
// //     result = dsigdbetadQ2dW2dz_total_qqg_checked(array[0], Q2, array[2], z);
// //     result *= Q2;   // Jacobian log(Q2)->Q2
// //     return log(result);
// // }

// // double InclusiveDiffractiveCrossSections::dsigdbetadQ2dW2dz_total_qqg_checked(double beta, double Q2, double W2, double z){
// //     //
// //     //  Check if in kinematically allowed region
// //     //
// //     double MX2=Kinematics::MX2(Q2, beta, 0);
// //     if(mCheckKinematics && z<beta) {
// //         if (mSettings->verboseLevel() > 2)
// //             cout<<"z<beta, beta="<<beta<<", z="<<z<<endl;
// //         return 0; // For QQG beta < z < 1
// //     }
// //     if (mCheckKinematics && !Kinematics::valid(mS, beta, Q2, W2, z, sqrt(MX2), false, (mSettings->verboseLevel() > 2) )) {
// //         if (mSettings->verboseLevel() > 2)
// //             cout << "InclusiveDiffractiveCrossSectionsIntegrals::dsigdbetadQ2dW2dz_total_qqg_checked(): warning, beta=" << beta << ", Q2=" << Q2 << ", W=" << sqrt(W2) << ", z="<< z
// //                 << " is outside of kinematically allowed region. Return 0." << endl;
// //         return 0;
// //     }
// //     double result = dsigdbetadQ2dW2dz_qqg(beta, Q2, W2, z);
// //     //
// //     //  Quark Species
// //     //
// //     //
    
// //     bool isChosen=false;
// //     double cs_rejected=0;
// //     for(int i=0; i<4; i++){
// //         double R=mRandom->Uniform(result-cs_rejected);
// //         double csTi=mDsigdbetadQ2dWdz_T_qqg[i];
// //         if(R <= csTi){
// //             mQuarkSpecies=i;
// //             isChosen=true;
// //             break;
// //         }
// //         cs_rejected+=mDsigdbetadQ2dWdz_T_qqg[i];
// //     }
// //     if(!isChosen) mQuarkSpecies=4;
    
// //     // Print-out at high verbose levels
// //     //
// //     if (mSettings->verboseLevel() > 10) {     // Spinal Tap ;-)
// //         cout << "InclusiveDiffractiveCrossSectionsIntegrals::dsigdbetadQ2dW2dz_total_qqg_checked(): " << result;
// //         cout << " at beta=" << beta << ", Q2=" << Q2 << ", W=" << sqrt(W2) << ", z=" << z<< endl;
// //     }
// //     //
// //     // Check validity of return value
// //     //
// //     if (std::isnan(result)) {
// //         cout << "InclusiveDiffractiveCrossSectionsIntegrals::dsigdbetadQ2dW2dz_total_qqg_checked(): Error, return value = NaN at" << endl;
// //         cout << "                                            beta=" << beta << ", Q2=" << Q2 << ", W=" << sqrt(W2) << ", z=" << z << endl;
// //         result = 0;
// //     }
// //     if (std::isinf(result)) {
// //         cout << "InclusiveDiffractiveCrossSectionsIntegrals::dsigdbetadQ2dW2dz_total_qqg_checked(): Error, return value = inf at" << endl;
// //         cout << "                                            beta=" << beta << ", Q2=" << Q2 << ", W=" << sqrt(W2) << ", z=" << z << endl;
// //         result = 0;
// //     }
// //     if (result < 0) {
// //         cout << "InclusiveDiffractiveCrossSectionsIntegrals::dsigdbetadQ2dW2dz_total_qqg_checked(): Error, negative cross-section at" << endl;
// //         cout << "                                            beta=" << beta << ", Q2=" << Q2 << ", W=" << sqrt(W2) << ", z=" << endl;
// //         result = 0;
// //     }
// //     mTotalCS_qqg=result;
// //     return result; //nb/GeV4
// // }

// // double InclusiveDiffractiveCrossSections::dsigdbetadQ2dW2dz_qqg(double beta, double Q2, double W2, double z) {
// //     double result = 0;
// //     for(int i=0; i<4; i++){
// //     // int i = 1;
// //         setQuarkIndex(i);
// //         double mf = Settings::quarkMass(mQuarkIndex);
// //         double Mqq2=(z/beta-1)*Q2;
// //         if(Mqq2<4*mf*mf) continue; //Not enough phase-space for the q and qbar
// //         double tmpresult = dsigmadbetadz_QQG(beta, Q2, W2, z); //nb
// //         if (mSettings->applyPhotonFlux()) tmpresult *= mPhotonFlux(Q2,W2,transverse); //nb/GeV4
// //         mDsigdbetadQ2dWdz_T_qqg[i]=tmpresult;
// //         // cout << "beta=" << beta << ", Q2=" << Q2 << ", W=" << sqrt(W2) << ", z=" << z << ", quark species=" << i << ", cross-section=" << tmpresult << " nb" << endl;
// //         result+=tmpresult;
// //     }
// //     return result; //nb/GeV4 (with flux) or nb (without)
// // }

// // ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// // //
// // //  InclusiveDiffractiveCrossSectionsIntegrals:
// // //  Inheriting class from InclusiveDiffractiveCrossSections
// // //
// // ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// // InclusiveDiffractiveCrossSectionsIntegrals::InclusiveDiffractiveCrossSectionsIntegrals()
// // {
// //     //Set up the integrals
// //     //Amplitude0:
// //     if(Amp0) delete Amp0;
// //     Amp0=new TF1("Amp0", this, &InclusiveDiffractiveCrossSections::uiAmplitude_0, 0, 30., 5);
    
// //     if(WFAmp0) delete WFAmp0;
// //     WFAmp0=new ROOT::Math::WrappedTF1(*Amp0);
    
// //     GIAmp0.SetFunction(*WFAmp0);
// //     GIAmp0.SetRelTolerance(1e-8);
    

// //     //Amplitude1:
// //     if(Amp1) delete Amp1;
// //     Amp1=new TF1("Amp1", this, &InclusiveDiffractiveCrossSections::uiAmplitude_1, 0, 30. , 5);
    
// //     // if(WFAmp1) delete WFAmp1;
// //     WFAmp1=new ROOT::Math::WrappedTF1(*Amp1);
    
// //     // GIAmp1.SetFunction(*WFAmp1);
// //     GIAmp1.SetRelTolerance(1e-8);
    
// //     //AmplitudeQQG:
// //     if(Ampqqg) delete Ampqqg;
// //     Ampqqg=new TF1("Ampqqg", this, &InclusiveDiffractiveCrossSections::uiAmplitude_qqg, 0, 30., 4);
    
// //     // if(WFAmpqqg) delete WFAmpqqg;
// //     WFAmpqqg=new ROOT::Math::WrappedTF1(*Ampqqg);
    
// //     GIAmpqqg.SetFunction(*WFAmpqqg);
// //     GIAmpqqg.SetRelTolerance(1e-8);

// // }

// // InclusiveDiffractiveCrossSectionsIntegrals::~InclusiveDiffractiveCrossSectionsIntegrals(){
// //     if(WFAmp0) delete WFAmp0;
// //     if(Amp0) delete Amp0;
// //     if(WFAmp1) delete WFAmp1;
// //     if(Amp1) delete Amp1;
// //     if(Ampqqg) delete Ampqqg;
// // }

// // double InclusiveDiffractiveCrossSectionsIntegrals::dsigmadbetadz_T(double beta, double Q2, double W2, double z) {
// //     double MX2 = Q2*(1-beta)/beta;
// //     double xpom = Kinematics::xpomeron(0., Q2, W2, sqrt(MX2));
// //     double mf = Settings::quarkMass(mQuarkIndex);
// //     double sqrtArg = 1.-4.*mf*mf/MX2;
// //     if (sqrtArg<0){
// //         if (mSettings->verboseLevel() > 2)
// //             cout<<"There is no phase-space for z, return 0."<<endl;
// //         return 0;
// //     }
// //     double z0 = (1.-sqrt(sqrtArg))/2.;
// //     if (z<z0 or z>1-z0){
// //         if (mSettings->verboseLevel() > 2)
// //             cout<<"z="<<z<<" which is smaller than z0="<<z0<<"or larger than (1-z0)="<<1-z0<<", return 0."<<endl;
// //         return 0;
// //     }
// //     double pt2 = z*(1-z)*Q2-mf*mf;
// //     if (pt2<0){
// //         if (mSettings->verboseLevel() > 2)
// //             cout<<"Not enough phase-space for quark transverse momenta, return 0. mf="<<mf<<endl;
// //         return 0;
// //     }
// //     if(z*(1-z)*MX2-mf*mf<0){ //kappa2<0
// //         if (mSettings->verboseLevel() > 2)
// //             cout<<"Not enough phase-space for quark production, return 0."<<endl;
// //         return 0;
// //     }
// //     double epsilon2=z*(1-z)*Q2+mf*mf;
// //     double ef=quarkCharge[mQuarkIndex];

// //     double Phi0=Phi_0(beta, xpom, z, Q2, mf);
// //     double Phi1=Phi_1(beta, xpom, z, Q2, mf);

// //     double prefactor=Nc*Q2*alpha_em/(4.*M_PI*beta*beta)*ef*ef*z*(1-z); //GeV2
// //     prefactor/=2.; //expanded z-range
// //     double term1=epsilon2*(z*z+(1-z)*(1-z))*Phi1; //GeV2*fm6
// //     double term2=mf*mf*Phi0; //GeV2*fm6
    
// //     double result=prefactor*(term1+term2); //GeV4*fm6
// //     result/=hbarc2*hbarc2; //fm2
// //     result *= 1e7; //nb
// //     return result;//nb
// // }

// // double InclusiveDiffractiveCrossSectionsIntegrals::dsigmadbetadz_L(double beta, double Q2, double W2, double z){
    
// //     double MX2 = Q2*(1-beta)/beta;
// //     double xpom = Kinematics::xpomeron(0., Q2, W2, sqrt(MX2));
// //     double mf =  Settings::quarkMass(mQuarkIndex);
// //     double pt2 = z*(1-z)*Q2-mf*mf;
// //     if (pt2<0){
// //         if (mSettings->verboseLevel() > 2)
// //             cout<<"Not enough phase-space for quark transverse momenta, return 0. mf="<<mf<<endl;
// //         return 0;
// //     }
// //     double sqrtArg=1.-4.*mf*mf/MX2;
// //     if (sqrtArg<0){
// //         if (mSettings->verboseLevel() > 2)
// //             cout<<"There is no phase-space for z, return 0."<<endl;
// //         return 0;
// //     }
// //     double z0=(1.-sqrt(sqrtArg))/2.;
// //     if (z<z0 or z>1-z0){
// //         if (mSettings->verboseLevel() > 2)
// //             cout<<"z="<<z<<" which is smaller than z0="<<z0<<"or larger than (1-z0)="<<1-z0<<", return 0."<<endl;
// //         return 0;
// //     }
// //     if(z*(1-z)*MX2-mf*mf<0){ //kappa2<0
// //         if (mSettings->verboseLevel() > 2)
// //             cout<<"Not enough phase-space for quark production, return 0."<<endl;
// //         return 0;
// //     }
// //     double ef=quarkCharge[mQuarkIndex];
// //     double Phi0=Phi_0(beta, xpom, z, Q2, mf); //fm6
    
// //     double term=Nc*Q2*Q2*alpha_em/(M_PI*beta*beta); //GeV4
// //     term*=ef*ef*z*z*z*(1-z)*(1-z)*(1-z);
// //     term*=Phi0; //fm6*GeV4
// //     term/=2.; //expanded z-range
// //     double result=term;
// //     result/=hbarc2*hbarc2; //fm2
// //     result *= 1e7; //nb
// //     return result; //nb
// // }

// // double InclusiveDiffractiveCrossSectionsIntegrals::Phi_0(double beta, double xpom, double z, double Q2, double mf){
// //     TF1* Phi0=new TF1("Phi0", this, &InclusiveDiffractiveCrossSectionsIntegrals::uiPhi_0, 0, 30., 0);
// //     ROOT::Math::WrappedTF1* WFPhi0=new ROOT::Math::WrappedTF1(*Phi0);
// //     ROOT::Math::GaussIntegrator GIPhi0;
// //     GIPhi0.SetFunction(*WFPhi0);
// //     GIPhi0.SetRelTolerance(1e-6);
    
// //     Amp0->SetParameter(0, beta);
// //     Amp0->SetParameter(1, xpom);
// //     Amp0->SetParameter(2, z);
// //     Amp0->SetParameter(3, Q2);
// //     Amp0->SetParameter(5, mf);

// //     if(mA>1)
// //         dipoleModel()->createSigma_ep_LookupTable(xpom);

// //     double blow=0;
// //     double result=GIPhi0.IntegralUp(blow); //fm6
// //     delete Phi0;
// //     delete WFPhi0;
// //     return result;
// // }

// // double InclusiveDiffractiveCrossSectionsIntegrals::Phi_1(double beta, double xpom, double z, double Q2, double mf){
// //     TF1* Phi1=new TF1("Phi1", this, &InclusiveDiffractiveCrossSectionsIntegrals::uiPhi_1, 0, 30., 0);
// //     ROOT::Math::WrappedTF1* WFPhi1=new ROOT::Math::WrappedTF1(*Phi1);
// //     ROOT::Math::GaussIntegrator GIPhi1;
// //     GIPhi1.SetFunction(*WFPhi1);
// //     GIPhi1.SetRelTolerance(1e-6);
        
// //     Amp1->SetParameter(0, beta);
// //     Amp1->SetParameter(1, xpom);
// //     Amp1->SetParameter(2, z);
// //     Amp1->SetParameter(3, Q2);
// //     Amp1->SetParameter(5, mf);
    
// //     if(mA>1)
// //         dipoleModel()->createSigma_ep_LookupTable(xpom);

// //     double blow=0;
// //     double result=GIPhi1.IntegralUp(blow); //fm6
// //     delete Phi1;
// //     delete WFPhi1;
// //     return result;
// // }

// // double InclusiveDiffractiveCrossSectionsIntegrals::uiPhi_0(double *var, double*){
// //     double b=*var;
    
// //     double amplitude=Amplitude_0(b); //fm2

// //     double result=2*M_PI*b*amplitude*amplitude; //fm5
// //     if(std::isnan(result) || std::isinf(result)) return 0.;
// //     return result;
// // }

// // double InclusiveDiffractiveCrossSectionsIntegrals::uiPhi_1(double *var, double*){
// //     double b=*var; //fm

// //     double amplitude=Amplitude_1(b); //fm2
// //     double result=2*M_PI*b*amplitude*amplitude; //fm5
// //     if(std::isnan(result) || std::isinf(result)) return 0.;
// //     return result;
// // }

// // double InclusiveDiffractiveCrossSectionsIntegrals::Amplitude_0(double b){
// //     //Gauss Integrator defined in header file and set in constructor

// //     Amp0->SetParameter(4, b);
    
// //     double rlow=1e-3;
// //     double rhigh=100;
// //     // double result=GIAmp0.IntegralUp(rlow); //fm2
// //     double result=GIAmp0.Integral(rlow,rhigh); //fm2
// //     if(std::isnan(result) || std::isinf(result)) return 0.;
// //     return result;
// // }


// // double InclusiveDiffractiveCrossSectionsIntegrals::Amplitude_1(double b){
// //     //Gauss Integrator defined in header file and set in constructor
// //     Amp1->SetParameter(4, b);
    
// //     double rlow=1e-3;
// //     double rhigh=100;
// //     // double result=GIAmp1.IntegralUp(rlow); //fm2
// //     double result=GIAmp1.Integral(rlow,rhigh); //fm2
// //     if(std::isnan(result) || std::isinf(result)) return 0.;
// //     return result;
// // }

// // double InclusiveDiffractiveCrossSections::uiAmplitude_0(double *var, double *par){
// //     double r=*var;
// //     if(r>100.) return 0; //IntegralUp sometimes tries too large values that break the dipole model
// //     double beta=par[0];
// //     double xpom=par[1];
// //     double z=par[2];
// //     double Q2=par[3];
// //     double b=par[4];
// //     double mf=par[5];
// //     double MX2=Q2*(1-beta)/beta;
// //     double kappa2=z*(1-z)*MX2-mf*mf;
// //     double kappa=sqrt(kappa2);
// //     double epsilon2=z*(1-z)*Q2+mf*mf;
// //     double epsilon=sqrt(epsilon2); //GeV
// //     double besselK0=TMath::BesselK0(epsilon*r/hbarc);
// //     double besselJ0=TMath::BesselJ0(kappa*r/hbarc);
// //     double dsigmadb2=0;
// //     if(mA==1)
// //         dsigmadb2=mDipoleModel->dsigmadb2ep(r, b, xpom);
// //     else{
// //         dsigmadb2=mDipoleModel->coherentDsigmadb2(r, b, xpom);
// //     }
// //     double result = r*besselK0*besselJ0*dsigmadb2; //fm
// //     if (std::isnan(result) || std::isinf(result)) return 0.;
// //     return result;
// // }

// // double InclusiveDiffractiveCrossSections::uiAmplitude_1(double *var, double *par){
// //     double r=*var; //fm
// //     if(r>100.) return 0; //IntegralUp sometimes tries too large values that break the dipole model
// //     double beta=par[0];
// //     double xpom=par[1];
// //     double z=par[2];
// //     double Q2=par[3]; //GeV2
// //     double b=par[4];
// //     double mf=par[5];
// //     double MX2=Q2*(1-beta)/beta; //GeV2
// //     double kappa2=z*(1-z)*MX2-mf*mf; //GeV2
// //     double kappa=sqrt(kappa2); //GeV
// //     double epsilon2=z*(1-z)*Q2+mf*mf;
// //     double epsilon=sqrt(epsilon2); //GeV
// //     double besselK1=TMath::BesselK1(epsilon*r/hbarc);
// //     double besselJ1=TMath::BesselJ1(kappa*r/hbarc);
// //     double dsigmadb2=0; //GeV0
// //     if(mA==1)
// //         dsigmadb2=mDipoleModel->dsigmadb2ep(r, b, xpom);
// //     else
// //         dsigmadb2=mDipoleModel->coherentDsigmadb2(r, b, xpom);

// //     double result = r*besselK1*besselJ1*dsigmadb2; //fm
// //     if (std::isnan(result) || std::isinf(result)) return 0.;
// //     return result;
// // }

// // //===================================================
// // //           QQG Calculations
// // //===================================================

// // // double InclusiveDiffractiveCrossSectionsIntegrals::dsigmadbetadz_QQG(double beta, double Q2, double W2, double ztilde) {
// // //     double MX2 = Q2*(1-beta)/beta;
// // //     double xpom = Kinematics::xpomeron(0., Q2, W2, sqrt(MX2));
    
// // //     double Phi=Phi_qqg(xpom, ztilde, Q2);
    
// // //     double alpha_S=0.15;
// // //     double ef=quarkCharge[mQuarkIndex];
    
// // //     double betafactor=(1-beta/ztilde)*(1-beta/ztilde)+beta/ztilde*beta/ztilde;
// // //     double result=alpha_S*alpha_em/(2*M_PI*M_PI*Q2)*betafactor*ef*ef*Phi; //GeV-2
// // //     result*=hbarc2; //fm2
// // //     result *= 1e7; //nb
// // //     // cout<< " beta="<<beta<<", Q2="<<Q2<<", W2="<<W2<<", ztilde="<<ztilde<<", result="<<result<<endl;
// // //     if(std::isnan(result) || std::isinf(result)) return 0.;
// // //     return result; //nb
// // // }

// // // double InclusiveDiffractiveCrossSectionsIntegrals::Phi_qqg(double xpom, double z, double Q2){

// // //     Ampqqg->SetParameter(0, xpom);
// // //     Ampqqg->SetParameter(1, z);

// // //     if(mA>1)
// // //         dipoleModel()->createSigma_ep_LookupTable(xpom);

// // //     mQ2=Q2;

// // //     ROOT::Math::Functor wf(this, &InclusiveDiffractiveCrossSectionsIntegrals::uiPhi_qqg, 2);
// // //     ROOT::Math::IntegratorMultiDim ig(ROOT::Math::IntegrationMultiDim::kADAPTIVE);
// // //     ig.SetFunction(wf);
// // //     ig.SetAbsTolerance(0.);
// // //     ig.SetRelTolerance(1e-6);
    
// // //     double bRange=3. * dipoleModel()->nucleus()->radius();
// // //     double lo[2]={0.,1e-4}; //b, k2
// // //     double hi[2]={bRange, Q2}; //b, k2
// // //     double result = ig.Integral(lo, hi)/hbarc; //GeV0
// // //     if(std::isnan(result) || std::isinf(result)) return 0.;
// // //     return result; //GeV0
// // // }

// // // double InclusiveDiffractiveCrossSectionsIntegrals::uiPhi_qqg(const double* var) {
// // //     double b=var[0];
// // //     double k2=var[1];
// // //     double Q2=mQ2;
 
// // //     Ampqqg->SetParameter(2, sqrt(k2));
// // //     Ampqqg->SetParameter(3, b);
    
// // //     double rlow=1e-3;
// // //     double rhigh= 100;
// // //     // double amp_qqg=GIAmpqqg.IntegralUp(rlow); //fm2
// // //     double amp_qqg=GIAmpqqg.Integral(rlow,rhigh); //fm2
// // //     amp_qqg/=hbarc2; //GeV-2
    
// // //     double result = (2*M_PI)*b/hbarc*k2*k2*log(Q2/k2)*amp_qqg*amp_qqg; //GeV-1
// // //     if(std::isnan(result) || std::isinf(result)) return 0.;
// // //     return result; //GeV-1
// // // }

// // // double InclusiveDiffractiveCrossSections::uiAmplitude_qqg(double *var, double *par){
// // //     double r=*var; //fm
// // //     if(r>100.) return 0; //IntegralUp sometimes tries too large values that break the dipole model
// // //     double xpom=par[0];
// // //     double z=par[1];
// // //     double k=par[2]; //GeV
// // //     double b=par[3];
    
// // //     double besselK2=ROOT::Math::cyl_bessel_k(2, sqrt(z)*k*r/hbarc);
// // //     double besselJ2=ROOT::Math::cyl_bessel_j(2, sqrt(1-z)*k*r/hbarc);
// // //     double dsigmadb2=0; //GeV0
// // //     if(mA==1)
// // //         dsigmadb2=mDipoleModel->dsigmadb2ep(r, b, xpom);
// // //     else
// // //         dsigmadb2=mDipoleModel->coherentDsigmadb2(r, b, xpom);
// // //     double term=1-0.5*dsigmadb2;
// // //     double dsigmadb2tilde=2*(1-term*term);
// // //     double result=r*besselK2*besselJ2*dsigmadb2tilde; //fm
// // //     if (std::isnan(result) || std::isinf(result)) {
// // //         return 0.0;
// // //     }   
// // //     return result;
// // // }

 
// // //========================================================================================== 
// // // bpn : testing new integration method implementation
// // //==========================================================================================

// // double InclusiveDiffractiveCrossSectionsIntegrals::dsigmadbetadz_QQG(double beta, double Q2, double W2, double ztilde) {
// //     double MX2 = Q2*(1-beta)/beta;
// //     double xpom = Kinematics::xpomeron(0., Q2, W2, sqrt(MX2));
    
// //     double Phi=Phi_qqg(xpom, ztilde, Q2);
    
// //     double alpha_S=0.15;
// //     double ef=quarkCharge[mQuarkIndex];
    
// //     double betafactor=(1-beta/ztilde)*(1-beta/ztilde)+beta/ztilde*beta/ztilde;
// //     double result=alpha_S*alpha_em/(2*M_PI*M_PI*Q2)*betafactor*ef*ef*Phi; //GeV-2
// //     result*=hbarc2; //fm2
// //     result *= 1e7; //nb
// //     // cout << " beta=" << beta << ", Q2=" << Q2 << ", W2=" << W2 << ", ztilde=" << ztilde << ", result=" << result << endl;
// //     // if(std::isnan(result) || std::isinf(result)) return 0.;
// //     return result; //nb
// // }

// // // C-style wrapper replacing ROOT::Math::Functor
// // // static int Cuhre_Integrand_QQG(const int *ndim, const double x[], const int *ncomp, double f[], void *userdata) {
// // //     auto* self = static_cast<InclusiveDiffractiveCrossSectionsIntegrals*>(userdata);
// // //     f[0] = self->uiPhi_qqg(x); // Pass normalized x[0], x[1] in [0,1]
// // //     return 0;
// // // }

// // double InclusiveDiffractiveCrossSectionsIntegrals::Phi_qqg(double xpom, double z, double Q2){

// //     Ampqqg->SetParameter(0, xpom);
// //     Ampqqg->SetParameter(1, z);

// //     if(mA>1)
// //         dipoleModel()->createSigma_ep_LookupTable(xpom);

// //     mQ2=Q2;

// //     int nregions, neval, fail;
// //     double integral[1], error[1], prob[1];

// //     // Cuhre replacement for ROOT::Math::IntegratorMultiDim
// //     // Parameters: ndim=2, ncomp=1, relTol=1e-4, maxeval=500000, key=9
// //     Cuhre(2, 1, Cuhre_Integrand_QQG, this, 1, 1e-4, 0., 0, 1000, 500000, 9,
// //           NULL, NULL, &nregions, &neval, &fail, integral, error, prob);

// //     double result = integral[0]/hbarc; //GeV0
// //     if(fail != 0 || std::isnan(result) || std::isinf(result)) return 0.;
// //     return result; //GeV0
// // }

// // double InclusiveDiffractiveCrossSectionsIntegrals::uiPhi_qqg(const double* var) {
// //     // Map Cuba's x[0], x[1] in [0,1] to physical bounds (b, k2)
// //     double bRange = 3. * dipoleModel()->nucleus()->radius();
// //     double b = var[0] * bRange;
// //     double k2 = 1e-4 + var[1] * (mQ2 - 1e-4);
// //     double jacobian = bRange * (mQ2 - 1e-4);

// //     double Q2=mQ2;
 
// //     Ampqqg->SetParameter(2, sqrt(k2));
// //     Ampqqg->SetParameter(3, b);
    
// //     GIAmpqqg.SetAbsTolerance(0.);

// //     double rlow=1e-3;
// //     double rhigh= 100;
// //     double amp_qqg=GIAmpqqg.Integral(rlow,rhigh); //fm2
// //     amp_qqg/=hbarc2; //GeV-2
    
// //     double result = (2*M_PI)*b/hbarc*k2*k2*log(Q2/k2)*amp_qqg*amp_qqg; //GeV-1
// //     result *= jacobian; // Include Jacobian for coordinate change

// //     // if(std::isnan(result) || std::isinf(result)) return 0.;
// //     return result; //GeV-1
    
// // }

// // double InclusiveDiffractiveCrossSections::uiAmplitude_qqg(double *var, double *par){
// //     double r=*var; //fm
// //     if(r>100.) return 0; //IntegralUp sometimes tries too large values that break the dipole model
// //     double xpom=par[0];
// //     double z=par[1];
// //     double k=par[2]; //GeV
// //     double b=par[3];
    
// //     double argK = sqrt(z)*k*r/hbarc;
// //     if (argK < 1e-10) return 0.; // Prevent Bessel divergence

// //     double besselK2=ROOT::Math::cyl_bessel_k(2, argK);
// //     double besselJ2=ROOT::Math::cyl_bessel_j(2, sqrt(1-z)*k*r/hbarc);
// //     double dsigmadb2=0; //GeV0
// //     if(mA==1)
// //         dsigmadb2=mDipoleModel->dsigmadb2ep(r, b, xpom);
// //     else
// //         dsigmadb2=mDipoleModel->coherentDsigmadb2(r, b, xpom);
// //     double term=1-0.5*dsigmadb2;
// //     double dsigmadb2tilde=2*(1-term*term);
// //     double result=r*besselK2*besselJ2*dsigmadb2tilde; //fm
// //     // if (std::isnan(result) || std::isinf(result) || std::abs(result) > 1e10) {
// //     //     return 0.0;
// //     // }   
// //     return result;
// // }

// // int InclusiveDiffractiveCrossSectionsIntegrals::Cuhre_Integrand_QQG(
// //         const int *ndim, const double x[], const int *ncomp, double f[], void *userdata) {
// //     auto* self = static_cast<InclusiveDiffractiveCrossSectionsIntegrals*>(userdata);
// //     f[0] = self->uiPhi_qqg(x);
// //     return 0;
// // }


// ////==========================================================================================
// // bpn : new improved script
// ///==========================================================================================

// ////==========================================================================================
// // bpn : new improved script
// ///==========================================================================================

// //==============================================================================
// //  InclusiveDiffractiveCrossSection.cpp
// //
// //  Copyright (C) 2024 Tobias Toll and Thomas Ullrich
// //
// //  This file is part of Sartre.
// //
// //  This program is free software: you can redistribute it and/or modify
// //  it under the terms of the GNU General Public License as published by
// //  the Free Software Foundation.
// //  This program is distributed in the hope that it will be useful,
// //  but without any warranty; without even the implied warranty of
// //  merchantability or fitness for a particular purpose. See the
// //  GNU General Public License for more details.
// //  You should have received a copy of the GNU General Public License
// //  along with this program.  If not, see <http://www.gnu.org/licenses/>.
// //
// //  Author: Tobias Toll
// //  Last update:
// //  $Date: 2024-06-03 11:27:05 -0400 (Mon, 03 Jun 2024) $
// //  $Author: ullrich $
// //==============================================================================
// #include "Math/SpecFunc.h"
// #include "TFile.h"
// #include <iostream>
// #include <cmath>
// #include <algorithm>
// #include <algorithm>
// #include <limits>
// #include "Constants.h"
// #include "Nucleus.h"
// #include "DipoleModel.h"
// #include "AlphaStrong.h"
// #include "Math/IntegratorMultiDim.h"
// #include "Math/Functor.h"
// #include "TMath.h"
// #include "WaveOverlap.h"
// #include "Kinematics.h"
// //#include "TableGeneratorSettings.h"
// #include "Settings.h"
// #include "Enumerations.h"
// #include "TF1.h"
// #include "TH1F.h"
// #include "cuba.h"
// #include "InclusiveDiffractiveCrossSections.h"
// #include "DglapEvolution.h"
// #include "Math/WrappedTF1.h"
// #include "Math/GaussIntegrator.h"
// #include "DipoleModelParameters.h"
// #include <unordered_map>


// #include "Math/GaussLegendreIntegrator.h"
// #include "Math/Functor.h"   // ROOT::Math::Functor1D

// #define PR(x) cout << #x << " = " << (x) << endl;

// //------------------------------------------------------------------------------
// // Persistent integrator caches for Phi_0()/Phi_1().
// //
// // These two functions used to `new`/`delete` a TF1 + WrappedTF1 +
// // GaussIntegrator triple EVERY time they were called. Since they're called
// // once per quark species per kinematic point (i.e. potentially millions of
// // times over a table-generation run), that repeated allocation/ROOT-object
// // construction was the dominant cost, far outweighing the integrator
// // tolerance settings. Caching by `this` keeps behavior correct even if more
// // than one InclusiveDiffractiveCrossSectionsIntegrals instance exists.
// // thread_local so this stays safe if instances are ever used from multiple
// // threads.
// //------------------------------------------------------------------------------
// namespace {
//     /*
//     struct Phi0IntegratorCache {
//         ROOT::Math::GaussIntegrator* gi = nullptr;

//         // Memoization: dsigmadbetadz_T() and dsigmadbetadz_L() both call
//         // Phi_0() with identical (beta, xpom, z, Q2, mf) for a given quark
//         // species -- the longitudinal call was previously repeating the
//         // transverse call's expensive double integral from scratch. One
//         // slot per quark species (4) is enough to catch this since that's
//         // the maximum "distance" between a T call and its matching L call.
//         static constexpr int kCacheSize = 4;
//         double keyBeta[kCacheSize] = {};
//         double keyXpom[kCacheSize] = {};
//         double keyZ[kCacheSize] = {};
//         double keyQ2[kCacheSize] = {};
//         double keyMf[kCacheSize] = {};
//         double value[kCacheSize] = {};
//         bool filled[kCacheSize] = {false, false, false, false};
//         int nextSlot = 0;
//     };
//     struct Phi1IntegratorCache {
//         ROOT::Math::GaussIntegrator* gi = nullptr;
//         // Phi_1() is only ever called from dsigmadbetadz_T(), so it doesn't
//         // get the T/L duplication Phi_0 does -- no memoization needed here.
//     };
//     */
//    struct Phi0IntegratorCache {
//        // Memoization: dsigmadbetadz_T() and dsigmadbetadz_L() both call
//        // Phi_0() with identical (beta, xpom, z, Q2, mf) for a given quark
//        // species -- the longitudinal call was previously repeating the
//        // transverse call's expensive double integral from scratch. One
//        // slot per quark species (4) is enough to catch this since that's
//        // the maximum "distance" between a T call and its matching L call.
//        static constexpr int kCacheSize = 4;
//        double keyBeta[kCacheSize] = {};
//        double keyXpom[kCacheSize] = {};
//        double keyZ[kCacheSize] = {};
//        double keyQ2[kCacheSize] = {};
//        double keyMf[kCacheSize] = {};
//        double value[kCacheSize] = {};
//        bool filled[kCacheSize] = {false, false, false, false};
//        int nextSlot = 0;
//     };
// // Phi1IntegratorCache is no longer needed at all -- Phi_1 had no memoization
// // logic to begin with, so once `gi` moves to a class member there's nothing
// // left for a per-instance cache struct to hold.

//     // Pure C++ outer-integral integrands (no TF1/WrappedTF1), mirroring the
//     // Amp0_Integrand/Amp1_Integrand/AmpQQG_Integrand pattern used for the
//     // inner r-integrals. operator()(b) reproduces uiPhi_0/uiPhi_1 exactly.
//     /*
//     struct Phi0_Integrand {
//         InclusiveDiffractiveCrossSectionsIntegrals* self;
//         double beta, xpom, z, Q2, mf;
//         double operator()(double b) const {
//             double amplitude = self->Amplitude_0(b); //fm2
//             double result = 2*M_PI*b*amplitude*amplitude; //fm5
//             if (std::isnan(result) || std::isinf(result)) return 0.;
//             return result;
//         }
//     };

//     struct Phi1_Integrand {
//         InclusiveDiffractiveCrossSectionsIntegrals* self;
//         double beta, xpom, z, Q2, mf;
//         double operator()(double b) const {
//             double amplitude = self->Amplitude_1(b); //fm2
//             double result = 2*M_PI*b*amplitude*amplitude; //fm5
//             if (std::isnan(result) || std::isinf(result)) return 0.;
//             return result;
//         }
//     };
//     */
//    struct Phi0_Integrand {
//         InclusiveDiffractiveCrossSectionsIntegrals* self;
//         double beta, xpom, z, Q2, mf;
//         double operator()(double b) const {
//             double amplitude = self->Amplitude_0(b, beta, xpom, z, Q2, mf); //fm2
//             double result = 2*M_PI*b*amplitude*amplitude; //fm5
//             if (std::isnan(result) || std::isinf(result)) return 0.;
//             return result;
//         }
//     };
//     // Phi1_Integrand analogous, calling self->Amplitude_1(b, beta, xpom, z, Q2, mf)
//     struct Phi1_Integrand {
//         InclusiveDiffractiveCrossSectionsIntegrals* self;
//         double beta, xpom, z, Q2, mf;
//         double operator()(double b) const {
//             double amplitude = self->Amplitude_1(b, beta, xpom, z, Q2, mf); //fm2
//             double result = 2*M_PI*b*amplitude*amplitude; //fm5
//             if (std::isnan(result) || std::isinf(result)) return 0.;
//             return result;
//         }
//     };
//     // Phi_0/Phi_1/Phi_qqg all call dipoleModel()->createSigma_ep_LookupTable(xpom)
//     // for nuclear targets (mA>1). Within one kinematic point, that's called
//     // once per quark species (and once more for the longitudinal pass) with
//     // the *same* xpom every time. Cache by (instance, xpom) so it's only
//     // rebuilt when xpom actually changes.
//     struct LookupTableCache {
//         double lastXpom = std::numeric_limits<double>::quiet_NaN();
//         bool valid = false;
//     };


//     // Phi_qqg(xpom, z, Q2) doesn't depend on quark species -- only the ef^2
//     // prefactor and the Mqq2 phase-space cut in dsigdbetadQ2dW2dz_qqg do.
//     // The 4-species loop there calls dsigmadbetadz_QQG with identical
//     // (beta, Q2, W2, ztilde) back-to-back, so xpom/z/Q2 are bit-identical
//     // across those calls -- safe to cache exactly, no tolerance needed.
//     struct PhiQQGCache {
//         double xpom = std::numeric_limits<double>::quiet_NaN();
//         double z    = std::numeric_limits<double>::quiet_NaN();
//         double Q2   = std::numeric_limits<double>::quiet_NaN();
//         double value = 0.0;
//         bool filled = false;
//     };

//     thread_local std::unordered_map<const void*, LookupTableCache> gLookupTableCache;
//     thread_local PhiQQGCache gPhiQQGCache;
// }

// InclusiveDiffractiveCrossSections::InclusiveDiffractiveCrossSections(){
//     mSettings = EventGeneratorSettings::instance();
//     mRandom = mSettings->randomGenerator();
//     mS = Kinematics::s(mSettings->eBeam(), mSettings->hBeam());
//     // PR(mS)
//     mPhotonFlux.setS(mS);
//     mCheckKinematics = true;
//     mCrossSectionRatioLT = 0;
    
//     mQ2=0;
    
// //    TableGeneratorSettings* settings = TableGeneratorSettings::instance();
//     // if(mDipoleModel) delete mDipoleModel;
    
//     mDipoleModelParameterSet = mSettings->dipoleModelParameterSet();
//     DipoleModelType model=mSettings->dipoleModelType();
//     if (model==bSat) {
//         mDipoleModel = new DipoleModel_bSat(mSettings);
//     }
//     else if(model==bNonSat){
//         mDipoleModel = new DipoleModel_bNonSat(mSettings);
//     }
//     else {
//         cout << "Integrals::init(): Error, model not implemented: "<< model << endl;
//         exit(1);
//     }
//     mA=mSettings->A();

// }

// InclusiveDiffractiveCrossSections::~InclusiveDiffractiveCrossSections(){}

// double* InclusiveDiffractiveCrossSections::dsigdbetadQ2dWdz_T_total(){return mDsigdbetadQ2dWdz_T_total;}
// double* InclusiveDiffractiveCrossSections::dsigdbetadQ2dWdz_L_total(){return mDsigdbetadQ2dWdz_L_total;}
// double* InclusiveDiffractiveCrossSections::dsigdbetadQ2dWdz_T_qqg(){ return mDsigdbetadQ2dWdz_T_qqg;};

// //These three functions are unique for the sub-classes:
// double InclusiveDiffractiveCrossSections::dsigmadbetadz_T(double, double, double, double){ return 0; }
// double InclusiveDiffractiveCrossSections::dsigmadbetadz_L(double, double, double, double){ return 0; }
// double InclusiveDiffractiveCrossSections::dsigmadbetadz_QQG(double, double, double, double){return 0;}
// void InclusiveDiffractiveCrossSections::setTableCollection(TableCollection*){};



// void InclusiveDiffractiveCrossSections::setCheckKinematics(bool val) {mCheckKinematics = val;}

// void InclusiveDiffractiveCrossSections::setFockState(FockState val){
//     mFockState=val;
// }

// FockState InclusiveDiffractiveCrossSections::getFockState(){
//     return mFockState;
// }

// double InclusiveDiffractiveCrossSections::operator()(double MX2, double Q2, double W2, double z){
//     return dsigdMX2dQ2dW2dz_total_checked(MX2, Q2, W2, z);
// }

// double InclusiveDiffractiveCrossSections::operator()(const double* array){
//     double result=0;
//     if(mFockState==QQ or mFockState==ALL){
//         result+=dsigdbetadQ2dW2dz_total_checked(array[0], array[1], array[2], array[3]);
//     }
//     else if(mFockState==QQG or
//             mFockState==ALL){
//         result+=dsigdbetadQ2dW2dz_total_qqg_checked(array[0], array[1], array[2], array[3]);
//     }
//     else{
//         cout<<"InclusiveDiffractiveCrossSectionsIntegrals::operator(): Fockstate is invalid, stopping"<<endl;
//         exit(0);
//     }
//     return result;
// }

// double InclusiveDiffractiveCrossSections::unuranPDF(const double* array){
//     //
//     // array is: beta, log(Q2), W2, z
//     //
//     double result = 0;
//     double Q2 = exp(array[1]);
//     result = dsigdbetadQ2dW2dz_total_checked(array[0], Q2, array[2], array[3]);
//     result *= Q2;   // Jacobian
//     return log(result);
// }

// double InclusiveDiffractiveCrossSections::dsigdMX2dQ2dW2dz_total_checked(double MX2, double Q2, double W2, double z){
//     double beta=Q2/(Q2+MX2);
//     double jacobian=beta*beta/Q2;
//     double result= dsigdbetadQ2dW2dz_total_checked(beta, Q2, W2, z);
//     return jacobian*result;
// }

// double InclusiveDiffractiveCrossSections::dsigdbetadQ2dW2dz_total_checked(double beta, double Q2, double W2, double z){
//     double result = 0;
//     //
//     //  Check if in kinematically allowed region
//     double MX2=Kinematics::MX2(Q2, beta, 0);
//     if (mCheckKinematics && !Kinematics::valid(mS, beta, Q2, W2, z, sqrt(MX2), false, (mSettings->verboseLevel() > 2) )) {
//         if (mSettings->verboseLevel() > 2)
//             cout << "InclusiveDiffractiveCrossSectionsIntegrals::dsigdbetadQ2dW2dz_total_checked(): warning, beta=" << beta << ", Q2=" << Q2 << ", W=" << sqrt(W2) << ", z="<< z
//                 << " is outside of kinematically allowed region. Return 0." << endl;
//         return 0;
//     }
    
//     double csT = dsigdbetadQ2dW2dz_total(beta, Q2, W2, z, transverse);
//     double csL = dsigdbetadQ2dW2dz_total(beta, Q2, W2, z, longitudinal);
//     result = csT + csL;
//     mCrossSectionRatioLT=csL/csT;
//     //
//     //  Polarization
//     //
//     if (mRandom->Uniform(result) <= csT)
//         mPolarization = transverse;
//     else
//         mPolarization = longitudinal;
//     //
//     //  Quark Species
//     //
//     //
    
//     if(mPolarization == transverse){
//         bool isChosen=false;
//         double cs_rejected=0;
//         for(int i=0; i<4; i++){
//             double R=mRandom->Uniform(csT-cs_rejected);
//             double csTi=mDsigdbetadQ2dWdz_T_total[i];
//             if(R <= csTi){
//                 mQuarkSpecies=i;
//                 isChosen=true;
//                 break;
//             }
//             cs_rejected+=mDsigdbetadQ2dWdz_T_total[i];
//         }
//         if(!isChosen) mQuarkSpecies=4;
//     }
//     else{
//         bool isChosen=false;
//         double cs_rejected=0;
//         for(int i=0; i<4; i++){
//             double R=mRandom->Uniform(csL-cs_rejected);
//             double csLi=mDsigdbetadQ2dWdz_L_total[i];
//             if(R <= csLi){
//                 mQuarkSpecies=i;
//                 isChosen=true;
//                 break;
//             }
//             cs_rejected+=mDsigdbetadQ2dWdz_L_total[i];
//         }
//         if(!isChosen) mQuarkSpecies=4;
//     }
    
//     // Print-out at high verbose levels
//     //
//     if (mSettings->verboseLevel() > 10) {     // Spinal Tap ;-)
//         cout << "InclusiveDiffractiveCrossSectionsIntegrals::dsigdbetadQ2dW2dz_total_checked(): " << result;
//         cout << " at beta=" << beta << ", Q2=" << Q2 << ", W=" << sqrt(W2) << ", z=" << z;
//         cout << " (" << (mPolarization == transverse ? "transverse" : "longitudinal");
//         cout << ')' << endl;
//     }
//     //
//     // Check validity of return value
//     //
//     if (std::isnan(result)) {
//         cout << "InclusiveDiffractiveCrossSectionsIntegrals::dsigdbetadQ2dW2dz_total_checked(): Error, return value = NaN at" << endl;
//         cout << "                                            beta=" << beta << ", Q2=" << Q2 << ", W=" << sqrt(W2) << ", z=" << z << endl;
//         result = 0;
//     }
//     if (std::isinf(result)) {
//         cout << "InclusiveDiffractiveCrossSectionsIntegrals::dsigdbetadQ2dW2dz_total_checked(): Error, return value = inf at" << endl;
//         cout << "                                            beta=" << beta << ", Q2=" << Q2 << ", W=" << sqrt(W2) << ", z=" << z << endl;
//         result = 0;
//     }
//     if (result < 0) {
//         cout << "InclusiveDiffractiveCrossSectionsIntegrals::dsigdbetadQ2dW2dz_total_checked(): Error, negative cross-section at" << endl;
//         cout << "                                            beta=" << beta << ", Q2=" << Q2 << ", W=" << sqrt(W2) << ", z=" << endl;
//         result = 0;
//     }
//     mTotalCS=result;
//     return result; //nb/GeV4
// }

// double InclusiveDiffractiveCrossSections::dsigdbetadQ2dW2dz_total(double beta, double Q2, double W2, double z, GammaPolarization pol) {
//     double result = 0;
//     for(int i=0; i<4; i++){
//     // int i = 1;
//         setQuarkIndex(i);
//         double tmpresult = dsigdbetadz_total(beta, Q2, W2, z, pol); //nb
//         if (mSettings->applyPhotonFlux()) tmpresult *= mPhotonFlux(Q2,W2,pol); //nb/GeV4
//         if(pol == transverse)
//             mDsigdbetadQ2dWdz_T_total[i]=tmpresult;
//         if(pol == longitudinal)
//             mDsigdbetadQ2dWdz_L_total[i]=tmpresult;
//         result+=tmpresult;
//     }
//     return result; //nb/GeV4
// }

// double InclusiveDiffractiveCrossSections::dsigdbetadz_total(double beta, double Q2, double W2, double z, GammaPolarization pol) {
//     double result = 0;
//     if(pol==transverse){
//         result = dsigmadbetadz_T(beta, Q2, W2, z);
//     }
//     else if(pol==longitudinal){
//         result = dsigmadbetadz_L(beta, Q2, W2, z);
//     }
//     return result; //nb
// }


// double InclusiveDiffractiveCrossSections::crossSectionOfLastCall(){
//     return mTotalCS;
// }

// GammaPolarization InclusiveDiffractiveCrossSections::polarizationOfLastCall(){
//     return mPolarization;
// }

// unsigned int InclusiveDiffractiveCrossSections::quarkSpeciesOfLastCall(){
//     return mQuarkSpecies;
// }

// double InclusiveDiffractiveCrossSections::quarkMassOfLastCall(){
//     return Settings::quarkMass(mQuarkSpecies);
// }

// void InclusiveDiffractiveCrossSections::setQuarkIndex(unsigned int val){mQuarkIndex=val;}

// DipoleModel* InclusiveDiffractiveCrossSections::dipoleModel(){
//     return mDipoleModel;
// }

// double InclusiveDiffractiveCrossSections::crossSectionRatioLTOfLastCall() const {return mCrossSectionRatioLT;}


// //===================================================
// //           QQG Calculations
// //===================================================
// double InclusiveDiffractiveCrossSections::unuranPDF_qqg(const double* array){
//     //
//     // array is: beta, log(Q2), W2, z
//     //
//     double result = 0;
//     double Q2 = exp(array[1]);

//     double z=array[3];
//     result = dsigdbetadQ2dW2dz_total_qqg_checked(array[0], Q2, array[2], z);
//     result *= Q2;   // Jacobian log(Q2)->Q2
//     return log(result);
// }

// double InclusiveDiffractiveCrossSections::dsigdbetadQ2dW2dz_total_qqg_checked(double beta, double Q2, double W2, double z){
//     //
//     //  Check if in kinematically allowed region
//     //
//     double MX2=Kinematics::MX2(Q2, beta, 0);
//     if(mCheckKinematics && z<beta) {
//         if (mSettings->verboseLevel() > 2)
//             cout<<"z<beta, beta="<<beta<<", z="<<z<<endl;
//         return 0; // For QQG beta < z < 1
//     }
//     if (mCheckKinematics && !Kinematics::valid(mS, beta, Q2, W2, z, sqrt(MX2), false, (mSettings->verboseLevel() > 2) )) {
//         if (mSettings->verboseLevel() > 2)
//             cout << "InclusiveDiffractiveCrossSectionsIntegrals::dsigdbetadQ2dW2dz_total_qqg_checked(): warning, beta=" << beta << ", Q2=" << Q2 << ", W=" << sqrt(W2) << ", z="<< z
//                 << " is outside of kinematically allowed region. Return 0." << endl;
//         return 0;
//     }
//     double result = dsigdbetadQ2dW2dz_qqg(beta, Q2, W2, z);
//     //
//     //  Quark Species
//     //
//     //
    
//     bool isChosen=false;
//     double cs_rejected=0;
//     for(int i=0; i<4; i++){
//         double R=mRandom->Uniform(result-cs_rejected);
//         double csTi=mDsigdbetadQ2dWdz_T_qqg[i];
//         if(R <= csTi){
//             mQuarkSpecies=i;
//             isChosen=true;
//             break;
//         }
//         cs_rejected+=mDsigdbetadQ2dWdz_T_qqg[i];
//     }
//     if(!isChosen) mQuarkSpecies=4;
    
//     // Print-out at high verbose levels
//     //
//     if (mSettings->verboseLevel() > 10) {     // Spinal Tap ;-)
//         cout << "InclusiveDiffractiveCrossSectionsIntegrals::dsigdbetadQ2dW2dz_total_qqg_checked(): " << result;
//         cout << " at beta=" << beta << ", Q2=" << Q2 << ", W=" << sqrt(W2) << ", z=" << z<< endl;
//     }
//     //
//     // Check validity of return value
//     //
//     if (std::isnan(result)) {
//         cout << "InclusiveDiffractiveCrossSectionsIntegrals::dsigdbetadQ2dW2dz_total_qqg_checked(): Error, return value = NaN at" << endl;
//         cout << "                                            beta=" << beta << ", Q2=" << Q2 << ", W=" << sqrt(W2) << ", z=" << z << endl;
//         result = 0;
//     }
//     if (std::isinf(result)) {
//         cout << "InclusiveDiffractiveCrossSectionsIntegrals::dsigdbetadQ2dW2dz_total_qqg_checked(): Error, return value = inf at" << endl;
//         cout << "                                            beta=" << beta << ", Q2=" << Q2 << ", W=" << sqrt(W2) << ", z=" << z << endl;
//         result = 0;
//     }
//     if (result < 0) {
//         cout << "InclusiveDiffractiveCrossSectionsIntegrals::dsigdbetadQ2dW2dz_total_qqg_checked(): Error, negative cross-section at" << endl;
//         cout << "                                            beta=" << beta << ", Q2=" << Q2 << ", W=" << sqrt(W2) << ", z=" << endl;
//         result = 0;
//     }
//     mTotalCS_qqg=result;
//     return result; //nb/GeV4
// }

// double InclusiveDiffractiveCrossSections::dsigdbetadQ2dW2dz_qqg(double beta, double Q2, double W2, double z) {
//     double result = 0;
//     for(int i=0; i<4; i++){
//     // int i = 1;
//         setQuarkIndex(i);
//         double mf = Settings::quarkMass(mQuarkIndex);
//         double Mqq2=(z/beta-1)*Q2;
//         if(Mqq2<4*mf*mf) continue; //Not enough phase-space for the q and qbar
//         double tmpresult = dsigmadbetadz_QQG(beta, Q2, W2, z); //nb
//         if (mSettings->applyPhotonFlux()) tmpresult *= mPhotonFlux(Q2,W2,transverse); //nb/GeV4
//         mDsigdbetadQ2dWdz_T_qqg[i]=tmpresult;
//         // cout << "beta=" << beta << ", Q2=" << Q2 << ", W=" << sqrt(W2) << ", z=" << z << ", quark species=" << i << ", cross-section=" << tmpresult << " nb" << endl;
//         result+=tmpresult;
//     }
//     return result; //nb/GeV4 (with flux) or nb (without)
// }

// ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// //
// //  InclusiveDiffractiveCrossSectionsIntegrals:
// //  Inheriting class from InclusiveDiffractiveCrossSections
// //
// ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// /*
// InclusiveDiffractiveCrossSectionsIntegrals::InclusiveDiffractiveCrossSectionsIntegrals()
// {
//     //Set up the integrals
//     //Amplitude0:
//     if(Amp0) delete Amp0;
//     Amp0=new TF1("Amp0", this, &InclusiveDiffractiveCrossSections::uiAmplitude_0, 0, 30., 5);
    
//     if(WFAmp0) delete WFAmp0;
//     WFAmp0=new ROOT::Math::WrappedTF1(*Amp0);
    
//     GIAmp0.SetFunction(*WFAmp0);
//     GIAmp0.SetRelTolerance(1e-8);
//     GIAmp0.SetAbsTolerance(1e-15);
    

//     //Amplitude1:
//     if(Amp1) delete Amp1;
//     Amp1=new TF1("Amp1", this, &InclusiveDiffractiveCrossSections::uiAmplitude_1, 0, 30. , 5);
    
//     if(WFAmp1) delete WFAmp1;
//     WFAmp1=new ROOT::Math::WrappedTF1(*Amp1);
    
//     GIAmp1.SetFunction(*WFAmp1);   // BUGFIX: this was previously commented out, so GIAmp1
//                                     // (used by Amplitude_1()/Phi_1(), part of the transverse
//                                     // cross section) never had a valid integrand set.
//     GIAmp1.SetRelTolerance(1e-8);
//     GIAmp1.SetAbsTolerance(1e-15);
//     //AmplitudeQQG:
//     if(Ampqqg) delete Ampqqg;
//     Ampqqg=new TF1("Ampqqg", this, &InclusiveDiffractiveCrossSections::uiAmplitude_qqg, 0, 30., 4);
    
//     // if(WFAmpqqg) delete WFAmpqqg;
//     WFAmpqqg=new ROOT::Math::WrappedTF1(*Ampqqg);
    
//     GIAmpqqg.SetFunction(*WFAmpqqg);
//     GIAmpqqg.SetRelTolerance(1e-8);
//     GIAmpqqg.SetAbsTolerance(1e-15);

// }

// InclusiveDiffractiveCrossSectionsIntegrals::~InclusiveDiffractiveCrossSectionsIntegrals(){
//     if(WFAmp0) delete WFAmp0;
//     if(Amp0) delete Amp0;
//     if(WFAmp1) delete WFAmp1;
//     if(Amp1) delete Amp1;
//     if(Ampqqg) delete Ampqqg;
// }

// double InclusiveDiffractiveCrossSectionsIntegrals::dsigmadbetadz_T(double beta, double Q2, double W2, double z) {
//     double MX2 = Q2*(1-beta)/beta;
//     double xpom = Kinematics::xpomeron(0., Q2, W2, sqrt(MX2));
//     double mf = Settings::quarkMass(mQuarkIndex);
//     double sqrtArg = 1.-4.*mf*mf/MX2;
//     if (sqrtArg<0){
//         if (mSettings->verboseLevel() > 2)
//             cout<<"There is no phase-space for z, return 0."<<endl;
//         return 0;
//     }
//     double z0 = (1.-sqrt(sqrtArg))/2.;
//     if (z<z0 or z>1-z0){
//         if (mSettings->verboseLevel() > 2)
//             cout<<"z="<<z<<" which is smaller than z0="<<z0<<"or larger than (1-z0)="<<1-z0<<", return 0."<<endl;
//         return 0;
//     }
//     double pt2 = z*(1-z)*Q2-mf*mf;
//     if (pt2<0){
//         if (mSettings->verboseLevel() > 2)
//             cout<<"Not enough phase-space for quark transverse momenta, return 0. mf="<<mf<<endl;
//         return 0;
//     }
//     if(z*(1-z)*MX2-mf*mf<0){ //kappa2<0
//         if (mSettings->verboseLevel() > 2)
//             cout<<"Not enough phase-space for quark production, return 0."<<endl;
//         return 0;
//     }
//     double epsilon2=z*(1-z)*Q2+mf*mf;
//     double ef=quarkCharge[mQuarkIndex];

//     double Phi0=Phi_0(beta, xpom, z, Q2, mf);
//     double Phi1=Phi_1(beta, xpom, z, Q2, mf);

//     double prefactor=Nc*Q2*alpha_em/(4.*M_PI*beta*beta)*ef*ef*z*(1-z); //GeV2
//     prefactor/=2.; //expanded z-range
//     double term1=epsilon2*(z*z+(1-z)*(1-z))*Phi1; //GeV2*fm6
//     double term2=mf*mf*Phi0; //GeV2*fm6
    
//     double result=prefactor*(term1+term2); //GeV4*fm6
//     result/=hbarc2*hbarc2; //fm2
//     result *= 1e7; //nb
//     return result;//nb
// }

// double InclusiveDiffractiveCrossSectionsIntegrals::dsigmadbetadz_L(double beta, double Q2, double W2, double z){
    
//     double MX2 = Q2*(1-beta)/beta;
//     double xpom = Kinematics::xpomeron(0., Q2, W2, sqrt(MX2));
//     double mf =  Settings::quarkMass(mQuarkIndex);
//     double pt2 = z*(1-z)*Q2-mf*mf;
//     if (pt2<0){
//         if (mSettings->verboseLevel() > 2)
//             cout<<"Not enough phase-space for quark transverse momenta, return 0. mf="<<mf<<endl;
//         return 0;
//     }
//     double sqrtArg=1.-4.*mf*mf/MX2;
//     if (sqrtArg<0){
//         if (mSettings->verboseLevel() > 2)
//             cout<<"There is no phase-space for z, return 0."<<endl;
//         return 0;
//     }
//     double z0=(1.-sqrt(sqrtArg))/2.;
//     if (z<z0 or z>1-z0){
//         if (mSettings->verboseLevel() > 2)
//             cout<<"z="<<z<<" which is smaller than z0="<<z0<<"or larger than (1-z0)="<<1-z0<<", return 0."<<endl;
//         return 0;
//     }
//     if(z*(1-z)*MX2-mf*mf<0){ //kappa2<0
//         if (mSettings->verboseLevel() > 2)
//             cout<<"Not enough phase-space for quark production, return 0."<<endl;
//         return 0;
//     }
//     double ef=quarkCharge[mQuarkIndex];
//     double Phi0=Phi_0(beta, xpom, z, Q2, mf); //fm6
    
//     double term=Nc*Q2*Q2*alpha_em/(M_PI*beta*beta); //GeV4
//     term*=ef*ef*z*z*z*(1-z)*(1-z)*(1-z);
//     term*=Phi0; //fm6*GeV4
//     term/=2.; //expanded z-range
//     double result=term;
//     result/=hbarc2*hbarc2; //fm2
//     result *= 1e7; //nb
//     return result; //nb
// }

// double InclusiveDiffractiveCrossSectionsIntegrals::Phi_0(double beta, double xpom, double z, double Q2, double mf){
//     // Build (once) and reuse the TF1/WrappedTF1/GaussIntegrator instead of
//     // allocating them on every call -- see Phi0IntegratorCache comment above.
//     thread_local std::unordered_map<const void*, Phi0IntegratorCache> caches;
//     Phi0IntegratorCache& c = caches[this];
//     if (!c.fn) {
//         c.fn = new TF1("Phi0", this, &InclusiveDiffractiveCrossSectionsIntegrals::uiPhi_0, 0, 30., 0);
//         c.wf = new ROOT::Math::WrappedTF1(*c.fn);
//         c.gi = new ROOT::Math::GaussIntegrator();
//         c.gi->SetFunction(*c.wf);
//         c.gi->SetRelTolerance(1e-6);
//         c.gi->SetAbsTolerance(1e-12);
//     }

//     // Memoization: the longitudinal pass calls Phi_0 with the same inputs
//     // the transverse pass already used for this quark species -- skip
//     // recomputing the whole double integral if we've already got it.
//     for (int i = 0; i < Phi0IntegratorCache::kCacheSize; ++i) {
//         if (c.filled[i] && c.keyBeta[i]==beta && c.keyXpom[i]==xpom &&
//             c.keyZ[i]==z && c.keyQ2[i]==Q2 && c.keyMf[i]==mf) {
//             return c.value[i];
//         }
//     }

//     Amp0->SetParameter(0, beta);
//     Amp0->SetParameter(1, xpom);
//     Amp0->SetParameter(2, z);
//     Amp0->SetParameter(3, Q2);
//     Amp0->SetParameter(5, mf);

//     if(mA>1){
//         LookupTableCache& lc = gLookupTableCache[this];
//         if(!(lc.valid && lc.lastXpom==xpom)){
//             dipoleModel()->createSigma_ep_LookupTable(xpom);
//             lc.lastXpom = xpom;
//             lc.valid = true;
//         }
//     }

//     double blow=0;
//     double result=c.gi->IntegralUp(blow); //fm6

//     int slot = c.nextSlot;
//     c.keyBeta[slot]=beta; c.keyXpom[slot]=xpom; c.keyZ[slot]=z; c.keyQ2[slot]=Q2; c.keyMf[slot]=mf;
//     c.value[slot]=result; c.filled[slot]=true;
//     c.nextSlot = (slot+1) % Phi0IntegratorCache::kCacheSize;

//     return result;
// }

// double InclusiveDiffractiveCrossSectionsIntegrals::Phi_1(double beta, double xpom, double z, double Q2, double mf){
//     // Build (once) and reuse the TF1/WrappedTF1/GaussIntegrator instead of
//     // allocating them on every call -- see Phi1IntegratorCache comment above.
//     thread_local std::unordered_map<const void*, Phi1IntegratorCache> caches;
//     Phi1IntegratorCache& c = caches[this];
//     if (!c.fn) {
//         c.fn = new TF1("Phi1", this, &InclusiveDiffractiveCrossSectionsIntegrals::uiPhi_1, 0, 30., 0);
//         c.wf = new ROOT::Math::WrappedTF1(*c.fn);
//         c.gi = new ROOT::Math::GaussIntegrator();
//         c.gi->SetFunction(*c.wf);
//         c.gi->SetRelTolerance(1e-6);
//         c.gi->SetAbsTolerance(1e-12);
//     }

//     Amp1->SetParameter(0, beta);
//     Amp1->SetParameter(1, xpom);
//     Amp1->SetParameter(2, z);
//     Amp1->SetParameter(3, Q2);
//     Amp1->SetParameter(5, mf);

//     if(mA>1){
//         LookupTableCache& lc = gLookupTableCache[this];
//         if(!(lc.valid && lc.lastXpom==xpom)){
//             dipoleModel()->createSigma_ep_LookupTable(xpom);
//             lc.lastXpom = xpom;
//             lc.valid = true;
//         }
//     }

//     double blow=0;
//     double result=c.gi->IntegralUp(blow); //fm6
//     return result;
// }

// double InclusiveDiffractiveCrossSectionsIntegrals::uiPhi_0(double *var, double*){
//     double b=*var;
    
//     double amplitude=Amplitude_0(b); //fm2

//     double result=2*M_PI*b*amplitude*amplitude; //fm5
//     if(std::isnan(result) || std::isinf(result)) return 0.;
//     return result;
// }

// double InclusiveDiffractiveCrossSectionsIntegrals::uiPhi_1(double *var, double*){
//     double b=*var; //fm

//     double amplitude=Amplitude_1(b); //fm2
//     double result=2*M_PI*b*amplitude*amplitude; //fm5
//     if(std::isnan(result) || std::isinf(result)) return 0.;
//     return result;
// }

// double InclusiveDiffractiveCrossSectionsIntegrals::Amplitude_0(double b){
//     //Gauss Integrator defined in header file and set in constructor

//     Amp0->SetParameter(4, b);
    
//     double rlow=1e-3;
//     double rhigh=100;
//     // double result=GIAmp0.IntegralUp(rlow); //fm2
//     double result=GIAmp0.Integral(rlow,rhigh); //fm2
//     if(std::isnan(result) || std::isinf(result)) return 0.;
//     return result;
// }


// double InclusiveDiffractiveCrossSectionsIntegrals::Amplitude_1(double b){
//     //Gauss Integrator defined in header file and set in constructor
//     Amp1->SetParameter(4, b);
    
//     double rlow=1e-3;
//     double rhigh=100;
//     // double result=GIAmp1.IntegralUp(rlow); //fm2
//     double result=GIAmp1.Integral(rlow,rhigh); //fm2
//     if(std::isnan(result) || std::isinf(result)) return 0.;
//     return result;
// }

// double InclusiveDiffractiveCrossSections::uiAmplitude_0(double *var, double *par){
//     double r=*var;
//     if(r>100.) return 0; //IntegralUp sometimes tries too large values that break the dipole model
//     double beta=par[0];
//     double xpom=par[1];
//     double z=par[2];
//     double Q2=par[3];
//     double b=par[4];
//     double mf=par[5];
//     double MX2=Q2*(1-beta)/beta;
//     double kappa2=z*(1-z)*MX2-mf*mf;
//     double kappa=sqrt(kappa2);
//     double epsilon2=z*(1-z)*Q2+mf*mf;
//     double epsilon=sqrt(epsilon2); //GeV
//     double besselK0=TMath::BesselK0(epsilon*r/hbarc);
//     double besselJ0=TMath::BesselJ0(kappa*r/hbarc);
//     double dsigmadb2=0;
//     if(mA==1)
//         dsigmadb2=mDipoleModel->dsigmadb2ep(r, b, xpom);
//     else{
//         dsigmadb2=mDipoleModel->coherentDsigmadb2(r, b, xpom);
//     }
//     double result = r*besselK0*besselJ0*dsigmadb2; //fm
//     if (std::isnan(result) || std::isinf(result)) return 0.;
//     return result;
// }

// double InclusiveDiffractiveCrossSections::uiAmplitude_1(double *var, double *par){
//     double r=*var; //fm
//     if(r>100.) return 0; //IntegralUp sometimes tries too large values that break the dipole model
//     double beta=par[0];
//     double xpom=par[1];
//     double z=par[2];
//     double Q2=par[3]; //GeV2
//     double b=par[4];
//     double mf=par[5];
//     double MX2=Q2*(1-beta)/beta; //GeV2
//     double kappa2=z*(1-z)*MX2-mf*mf; //GeV2
//     double kappa=sqrt(kappa2); //GeV
//     double epsilon2=z*(1-z)*Q2+mf*mf;
//     double epsilon=sqrt(epsilon2); //GeV
//     double besselK1=TMath::BesselK1(epsilon*r/hbarc);
//     double besselJ1=TMath::BesselJ1(kappa*r/hbarc);
//     double dsigmadb2=0; //GeV0
//     if(mA==1)
//         dsigmadb2=mDipoleModel->dsigmadb2ep(r, b, xpom);
//     else
//         dsigmadb2=mDipoleModel->coherentDsigmadb2(r, b, xpom);

//     double result = r*besselK1*besselJ1*dsigmadb2; //fm
//     if (std::isnan(result) || std::isinf(result)) return 0.;
//     return result;
// }

// //===================================================
// //           QQG Calculations
// //===================================================

// double InclusiveDiffractiveCrossSectionsIntegrals::dsigmadbetadz_QQG(double beta, double Q2, double W2, double ztilde) {
//     double MX2 = Q2*(1-beta)/beta;
//     double xpom = Kinematics::xpomeron(0., Q2, W2, sqrt(MX2));
    
//     double Phi=Phi_qqg(xpom, ztilde, Q2);
    
//     double alpha_S=0.15;
//     double ef=quarkCharge[mQuarkIndex];
    
//     double betafactor=(1-beta/ztilde)*(1-beta/ztilde)+beta/ztilde*beta/ztilde;
//     double result=alpha_S*alpha_em/(2*M_PI*M_PI*Q2)*betafactor*ef*ef*Phi; //GeV-2
//     result*=hbarc2; //fm2
//     result *= 1e7; //nb
//     // cout<< " beta="<<beta<<", Q2="<<Q2<<", W2="<<W2<<", ztilde="<<ztilde<<", result="<<result<<endl;
//     if(std::isnan(result) || std::isinf(result)) return 0.;
//     return result; //nb
// }

// double InclusiveDiffractiveCrossSectionsIntegrals::Phi_qqg(double xpom, double z, double Q2){

//     if (gPhiQQGCache.filled &&
//         gPhiQQGCache.xpom == xpom &&
//         gPhiQQGCache.z    == z &&
//         gPhiQQGCache.Q2   == Q2) {
//         return gPhiQQGCache.value;
//     }

//     Ampqqg->SetParameter(0, xpom);
//     Ampqqg->SetParameter(1, z);

//     if(mA>1){
//         LookupTableCache& lc = gLookupTableCache[this];
//         if(!(lc.valid && lc.lastXpom==xpom)){
//             dipoleModel()->createSigma_ep_LookupTable(xpom);
//             lc.lastXpom = xpom;
//             lc.valid = true;
//         }
//     }

//     mQ2=Q2;

//     int nregions, neval, fail;
//     double integral[1], error[1], prob[1];

//     // Cuhre works on the unit cube [0,1]^2; uiPhi_qqg maps x[0],x[1] back
//     // to physical (b, k2) and applies the Jacobian internally.
//     // cubacores(0, 0);
//     Cuhre(2, 1, Cuhre_Integrand_QQG, this, 1, 1e-6, 1e-12, 0, 1000, 500000, 9,
//           NULL, NULL, &nregions, &neval, &fail, integral, error, prob);

//     double result = integral[0]/hbarc; //GeV0
//     if(fail != 0 || std::isnan(result) || std::isinf(result)) result = 0.;

//     gPhiQQGCache.xpom = xpom;
//     gPhiQQGCache.z    = z;
//     gPhiQQGCache.Q2   = Q2;
//     gPhiQQGCache.value = result;
//     gPhiQQGCache.filled = true;

//     return result;
// }

// double InclusiveDiffractiveCrossSectionsIntegrals::uiPhi_qqg(const double* var) {
//     // Map Cuba's x[0], x[1] in [0,1] to physical bounds (b, k2)
//     double bRange = 3. * dipoleModel()->nucleus()->radius();
//     double b  = var[0] * bRange;
//     double k2 = 1e-4 + var[1] * (mQ2 - 1e-4);
//     double jacobian = bRange * (mQ2 - 1e-4);

//     double Q2 = mQ2;

//     Ampqqg->SetParameter(2, sqrt(k2));
//     Ampqqg->SetParameter(3, b);

//     double rlow=1e-3;
//     double rhigh=100;
//     double amp_qqg=GIAmpqqg.Integral(rlow,rhigh); //fm2
//     amp_qqg/=hbarc2; //GeV-2

//     double result = (2*M_PI)*b/hbarc*k2*k2*log(Q2/k2)*amp_qqg*amp_qqg; //GeV-1
//     result *= jacobian; // Jacobian for the [0,1]^2 -> (b,k2) change of variables

//     if (std::isnan(result) || std::isinf(result)) return 0.;
//     return result; //GeV-1
// }

// int InclusiveDiffractiveCrossSectionsIntegrals::Cuhre_Integrand_QQG(
//         const int *ndim, const double x[], const int *ncomp, double f[], void *userdata) {
//     auto* self = static_cast<InclusiveDiffractiveCrossSectionsIntegrals*>(userdata);
//     f[0] = self->uiPhi_qqg(x);
//     return 0;
// }

// double InclusiveDiffractiveCrossSections::uiAmplitude_qqg(double *var, double *par){
//     double r=*var; //fm
//     if(r>100.) return 0; //IntegralUp sometimes tries too large values that break the dipole model
//     double xpom=par[0];
//     double z=par[1];
//     double k=par[2]; //GeV
//     double b=par[3];
    
//     double besselK2=ROOT::Math::cyl_bessel_k(2, sqrt(z)*k*r/hbarc);
//     double besselJ2=ROOT::Math::cyl_bessel_j(2, sqrt(1-z)*k*r/hbarc);
//     double dsigmadb2=0; //GeV0
//     if(mA==1)
//         dsigmadb2=mDipoleModel->dsigmadb2ep(r, b, xpom);
//     else
//         dsigmadb2=mDipoleModel->coherentDsigmadb2(r, b, xpom);
//     double term=1-0.5*dsigmadb2;
//     double dsigmadb2tilde=2*(1-term*term);
//     double result=r*besselK2*besselJ2*dsigmadb2tilde; //fm
//     if (std::isnan(result) || std::isinf(result)) {
//         return 0.0;
//     }   
//     return result;
// }
// */
 
// //========================================================================================== 
// // bpn : testing new integration method implementation
// //==========================================================================================

// namespace {

// // Pure C++ functors (no TF1/WrappedTF1 inheritance) fed directly to the
// // class-member GaussLegendreIntegrator via ROOT::Math::Functor1D.
// struct Amp0_Integrand {
//     InclusiveDiffractiveCrossSectionsIntegrals* self;
//     double beta, xpom, z, Q2, mf, b;
//     double operator()(double r) const {
//         return self->ComputeAmplitude0Core(r, b, beta, xpom, z, Q2, mf);
//     }
// };

// struct Amp1_Integrand {
//     InclusiveDiffractiveCrossSectionsIntegrals* self;
//     double beta, xpom, z, Q2, mf, b;
//     double operator()(double r) const {
//         return self->ComputeAmplitude1Core(r, b, beta, xpom, z, Q2, mf);
//     }
// };

// struct AmpQQG_Integrand {
//     InclusiveDiffractiveCrossSectionsIntegrals* self;
//     double xpom, z, k, b;
//     double operator()(double r) const {
//         return self->ComputeAmplitudeQQGCore(r, xpom, z, k, b);
//     }
// };

// } // namespace
// /*
// InclusiveDiffractiveCrossSectionsIntegrals::InclusiveDiffractiveCrossSectionsIntegrals()
// {
//     // GIAmp0/GIAmp1/GIAmpqqg are fixed-order (256-pt) Gauss-Legendre integrators
//     // (default-constructed via in-class member initializers in the header).
//     // No TF1/WrappedTF1 setup is needed anymore: the integrand functor is built
//     // and handed to SetFunction() fresh on every call in Amplitude_0/Amplitude_1/
//     // uiPhi_qqg below.
    
// }*/
// InclusiveDiffractiveCrossSectionsIntegrals::InclusiveDiffractiveCrossSectionsIntegrals()
// {
//     // Cuba's Cuhre() forks worker processes by default. Forking from inside
//     // an MPI rank (this object is constructed and used inside the MPI
//     // worker loop in main.cpp) is unsafe and was multiplying memory use by
//     // the fork pool size on every single Phi_qqg() call. cubacores(0,0)
//     // tells Cuba to run serially in-process instead of forking.
//     // cubacores(0, 0);

//     GIPhi0.SetRelTolerance(1e-5);
//     GIPhi0.SetAbsTolerance(1e-12);

//     GIPhi1.SetRelTolerance(1e-5);
//     GIPhi1.SetAbsTolerance(1e-12);

//     GIAmp0.SetRelTolerance(1e-7);
//     GIAmp0.SetAbsTolerance(1e-15);

//     GIAmp1.SetRelTolerance(1e-7);
//     GIAmp1.SetAbsTolerance(1e-15);

//     GIAmpqqg.SetRelTolerance(1e-7);
//     GIAmpqqg.SetAbsTolerance(1e-15);
// }

// InclusiveDiffractiveCrossSectionsIntegrals::~InclusiveDiffractiveCrossSectionsIntegrals(){
// }

// double InclusiveDiffractiveCrossSectionsIntegrals::dsigmadbetadz_T(double beta, double Q2, double W2, double z) {
//     double MX2 = Q2*(1-beta)/beta;
//     double xpom = Kinematics::xpomeron(0., Q2, W2, sqrt(MX2));
//     double mf = Settings::quarkMass(mQuarkIndex);
//     double sqrtArg = 1.-4.*mf*mf/MX2;
//     if (sqrtArg<0){
//         if (mSettings->verboseLevel() > 2)
//             cout<<"There is no phase-space for z, return 0."<<endl;
//         return 0;
//     }
//     double z0 = (1.-sqrt(sqrtArg))/2.;
//     if (z<z0 or z>1-z0){
//         if (mSettings->verboseLevel() > 2)
//             cout<<"z="<<z<<" which is smaller than z0="<<z0<<"or larger than (1-z0)="<<1-z0<<", return 0."<<endl;
//         return 0;
//     }
//     double pt2 = z*(1-z)*Q2-mf*mf;
//     if (pt2<0){
//         if (mSettings->verboseLevel() > 2)
//             cout<<"Not enough phase-space for quark transverse momenta, return 0. mf="<<mf<<endl;
//         return 0;
//     }
//     if(z*(1-z)*MX2-mf*mf<0){ //kappa2<0
//         if (mSettings->verboseLevel() > 2)
//             cout<<"Not enough phase-space for quark production, return 0."<<endl;
//         return 0;
//     }
//     double epsilon2=z*(1-z)*Q2+mf*mf;
//     double ef=quarkCharge[mQuarkIndex];

//     double Phi0=Phi_0(beta, xpom, z, Q2, mf);
//     double Phi1=Phi_1(beta, xpom, z, Q2, mf);

//     double prefactor=Nc*Q2*alpha_em/(4.*M_PI*beta*beta)*ef*ef*z*(1-z); //GeV2
//     prefactor/=2.; //expanded z-range
//     double term1=epsilon2*(z*z+(1-z)*(1-z))*Phi1; //GeV2*fm6
//     double term2=mf*mf*Phi0; //GeV2*fm6
    
//     double result=prefactor*(term1+term2); //GeV4*fm6
//     result/=hbarc2*hbarc2; //fm2
//     result *= 1e7; //nb
//     return result;//nb
// }

// double InclusiveDiffractiveCrossSectionsIntegrals::dsigmadbetadz_L(double beta, double Q2, double W2, double z){
    
//     double MX2 = Q2*(1-beta)/beta;
//     double xpom = Kinematics::xpomeron(0., Q2, W2, sqrt(MX2));
//     double mf =  Settings::quarkMass(mQuarkIndex);
//     double pt2 = z*(1-z)*Q2-mf*mf;
//     if (pt2<0){
//         if (mSettings->verboseLevel() > 2)
//             cout<<"Not enough phase-space for quark transverse momenta, return 0. mf="<<mf<<endl;
//         return 0;
//     }
//     double sqrtArg=1.-4.*mf*mf/MX2;
//     if (sqrtArg<0){
//         if (mSettings->verboseLevel() > 2)
//             cout<<"There is no phase-space for z, return 0."<<endl;
//         return 0;
//     }
//     double z0=(1.-sqrt(sqrtArg))/2.;
//     if (z<z0 or z>1-z0){
//         if (mSettings->verboseLevel() > 2)
//             cout<<"z="<<z<<" which is smaller than z0="<<z0<<"or larger than (1-z0)="<<1-z0<<", return 0."<<endl;
//         return 0;
//     }
//     if(z*(1-z)*MX2-mf*mf<0){ //kappa2<0
//         if (mSettings->verboseLevel() > 2)
//             cout<<"Not enough phase-space for quark production, return 0."<<endl;
//         return 0;
//     }
//     double ef=quarkCharge[mQuarkIndex];
//     double Phi0=Phi_0(beta, xpom, z, Q2, mf); //fm6
    
//     double term=Nc*Q2*Q2*alpha_em/(M_PI*beta*beta); //GeV4
//     term*=ef*ef*z*z*z*(1-z)*(1-z)*(1-z);
//     term*=Phi0; //fm6*GeV4
//     term/=2.; //expanded z-range
//     double result=term;
//     result/=hbarc2*hbarc2; //fm2
//     result *= 1e7; //nb
//     return result; //nb
// }
// /*
// double InclusiveDiffractiveCrossSectionsIntegrals::Phi_0(double beta, double xpom, double z, double Q2, double mf){
//     // Build (once) and reuse the TF1/WrappedTF1/GaussIntegrator instead of
//     // allocating them on every call -- see Phi0IntegratorCache comment above.
//     // (This is the outer b-integral for Phi_0 itself; unrelated to the inner
//     // r-integral refactor below, which only touches Amplitude_0/Amplitude_1/uiPhi_qqg.)
//     thread_local std::unordered_map<const void*, Phi0IntegratorCache> caches;
//     Phi0IntegratorCache& c = caches[this];
//     if (!c.fn) {
//         c.fn = new TF1("Phi0", this, &InclusiveDiffractiveCrossSectionsIntegrals::uiPhi_0, 0, 30., 0);
//         c.wf = new ROOT::Math::WrappedTF1(*c.fn);
//         c.gi = new ROOT::Math::GaussIntegrator();
//         c.gi->SetFunction(*c.wf);
//         c.gi->SetRelTolerance(1e-6);
//         c.gi->SetAbsTolerance(1e-12);
//     }

//     // Memoization: the longitudinal pass calls Phi_0 with the same inputs
//     // the transverse pass already used for this quark species -- skip
//     // recomputing the whole double integral if we've already got it.
//     for (int i = 0; i < Phi0IntegratorCache::kCacheSize; ++i) {
//         if (c.filled[i] && c.keyBeta[i]==beta && c.keyXpom[i]==xpom &&
//             c.keyZ[i]==z && c.keyQ2[i]==Q2 && c.keyMf[i]==mf) {
//             return c.value[i];
//         }
//     }

//     // Set the state Amplitude_0() will read when this outer b-integral
//     // drives calls into it (replaces the old Amp0->SetParameter(0..3,5,...)).
//     mAmp0Beta = beta;
//     mAmp0Xpom = xpom;
//     mAmp0Z    = z;
//     mAmp0Q2   = Q2;
//     mAmp0Mf   = mf;

//     if(mA>1){
//         LookupTableCache& lc = gLookupTableCache[this];
//         if(!(lc.valid && lc.lastXpom==xpom)){
//             dipoleModel()->createSigma_ep_LookupTable(xpom);
//             lc.lastXpom = xpom;
//             lc.valid = true;
//         }
//     }

//     double blow=0;
//     double result=c.gi->IntegralUp(blow); //fm6

//     int slot = c.nextSlot;
//     c.keyBeta[slot]=beta; c.keyXpom[slot]=xpom; c.keyZ[slot]=z; c.keyQ2[slot]=Q2; c.keyMf[slot]=mf;
//     c.value[slot]=result; c.filled[slot]=true;
//     c.nextSlot = (slot+1) % Phi0IntegratorCache::kCacheSize;

//     return result;
// }


// double InclusiveDiffractiveCrossSectionsIntegrals::Phi_1(double beta, double xpom, double z, double Q2, double mf){
//     // Build (once) and reuse the TF1/WrappedTF1/GaussIntegrator instead of
//     // allocating them on every call -- see Phi1IntegratorCache comment above.
//     thread_local std::unordered_map<const void*, Phi1IntegratorCache> caches;
//     Phi1IntegratorCache& c = caches[this];
//     if (!c.fn) {
//         c.fn = new TF1("Phi1", this, &InclusiveDiffractiveCrossSectionsIntegrals::uiPhi_1, 0, 30., 0);
//         c.wf = new ROOT::Math::WrappedTF1(*c.fn);
//         c.gi = new ROOT::Math::GaussIntegrator();
//         c.gi->SetFunction(*c.wf);
//         c.gi->SetRelTolerance(1e-6);
//         c.gi->SetAbsTolerance(1e-12);
//     }

//     // Set the state Amplitude_1() will read (replaces Amp1->SetParameter(0..3,5,...)).
//     mAmp1Beta = beta;
//     mAmp1Xpom = xpom;
//     mAmp1Z    = z;
//     mAmp1Q2   = Q2;
//     mAmp1Mf   = mf;

//     if(mA>1){
//         LookupTableCache& lc = gLookupTableCache[this];
//         if(!(lc.valid && lc.lastXpom==xpom)){
//             dipoleModel()->createSigma_ep_LookupTable(xpom);
//             lc.lastXpom = xpom;
//             lc.valid = true;
//         }
//     }

//     double blow=0;
//     double result=c.gi->IntegralUp(blow); //fm6
//     return result;
// }
// */
// double InclusiveDiffractiveCrossSectionsIntegrals::Phi_0(double beta, double xpom, double z, double Q2, double mf){
//     // Memoization cache (values only) is still per-(thread, instance) since
//     // multiple threads may run different instances -- GIPhi0 itself is now a
//     // plain member, same treatment as GIAmp0/GIAmp1 for the inner r-integral.
//     thread_local std::unordered_map<const void*, Phi0IntegratorCache> caches;
//     Phi0IntegratorCache& c = caches[this];

//     // Memoization: the longitudinal pass calls Phi_0 with the same inputs
//     // the transverse pass already used for this quark species -- skip
//     // recomputing the whole double integral if we've already got it.
//     for (int i = 0; i < Phi0IntegratorCache::kCacheSize; ++i) {
//         if (c.filled[i] && c.keyBeta[i]==beta && c.keyXpom[i]==xpom &&
//             c.keyZ[i]==z && c.keyQ2[i]==Q2 && c.keyMf[i]==mf) {
//             return c.value[i];
//         }
//     }

//     if(mA>1){
//         LookupTableCache& lc = gLookupTableCache[this];
//         if(!(lc.valid && lc.lastXpom==xpom)){
//             dipoleModel()->createSigma_ep_LookupTable(xpom);
//             lc.lastXpom = xpom;
//             lc.valid = true;
//         }
//     }

//     Phi0_Integrand integrand{this, beta, xpom, z, Q2, mf};
//     ROOT::Math::Functor1D functor(integrand);
//     GIPhi0.SetFunction(functor);

//     double blow=0;
//     double result=GIPhi0.IntegralUp(blow); //fm6

//     int slot = c.nextSlot;
//     c.keyBeta[slot]=beta; c.keyXpom[slot]=xpom; c.keyZ[slot]=z; c.keyQ2[slot]=Q2; c.keyMf[slot]=mf;
//     c.value[slot]=result; c.filled[slot]=true;
//     c.nextSlot = (slot+1) % Phi0IntegratorCache::kCacheSize;

//     return result;
// }

// double InclusiveDiffractiveCrossSectionsIntegrals::Phi_1(double beta, double xpom, double z, double Q2, double mf){
//     if(mA>1){
//         LookupTableCache& lc = gLookupTableCache[this];
//         if(!(lc.valid && lc.lastXpom==xpom)){
//             dipoleModel()->createSigma_ep_LookupTable(xpom);
//             lc.lastXpom = xpom;
//             lc.valid = true;
//         }
//     }

//     Phi1_Integrand integrand{this, beta, xpom, z, Q2, mf};
//     ROOT::Math::Functor1D functor(integrand);
//     GIPhi1.SetFunction(functor);

//     double blow=0;
//     double result=GIPhi1.IntegralUp(blow); //fm6
//     return result;
// }
// double InclusiveDiffractiveCrossSectionsIntegrals::Amplitude_0(
//         double b, double beta, double xpom, double z, double Q2, double mf){
//     Amp0_Integrand integrand{this, beta, xpom, z, Q2, mf, b};
//     ROOT::Math::Functor1D functor(integrand);
//     GIAmp0.SetFunction(functor);

//     double rlow=1e-3;
//     double rhigh=100;
//     double result=GIAmp0.Integral(rlow,rhigh); //fm2
//     if(std::isnan(result) || std::isinf(result)) return 0.;
//     return result;
// }

// double InclusiveDiffractiveCrossSectionsIntegrals::Amplitude_1(
//         double b, double beta, double xpom, double z, double Q2, double mf){
//     Amp1_Integrand integrand{this, beta, xpom, z, Q2, mf, b};
//     ROOT::Math::Functor1D functor(integrand);
//     GIAmp1.SetFunction(functor);

//     double rlow=1e-3;
//     double rhigh=100;
//     double result=GIAmp1.Integral(rlow,rhigh); //fm2
//     if(std::isnan(result) || std::isinf(result)) return 0.;
//     return result;
// }
// double InclusiveDiffractiveCrossSectionsIntegrals::ComputeAmplitude0Core(
//         double r, double b, double beta, double xpom, double z, double Q2, double mf) const {
//     if(r>100.) return 0; //Gauss nodes/IntegralUp sometimes probe values that break the dipole model
//     double MX2=Q2*(1-beta)/beta;
//     double kappa2=z*(1-z)*MX2-mf*mf;
//     double kappa=sqrt(kappa2);
//     double epsilon2=z*(1-z)*Q2+mf*mf;
//     double epsilon=sqrt(epsilon2); //GeV
//     double besselK0=TMath::BesselK0(epsilon*r/hbarc);
//     double besselJ0=TMath::BesselJ0(kappa*r/hbarc);
//     double dsigmadb2=0;
//     if(mA==1)
//         dsigmadb2=mDipoleModel->dsigmadb2ep(r, b, xpom);
//     else{
//         dsigmadb2=mDipoleModel->coherentDsigmadb2(r, b, xpom);
//     }
//     double result = r*besselK0*besselJ0*dsigmadb2; //fm
//     if (std::isnan(result) || std::isinf(result)) return 0.;
//     return result;
// }

// double InclusiveDiffractiveCrossSectionsIntegrals::ComputeAmplitude1Core(
//         double r, double b, double beta, double xpom, double z, double Q2, double mf) const {
//     if(r>100.) return 0; //Gauss nodes/IntegralUp sometimes probe values that break the dipole model
//     double MX2=Q2*(1-beta)/beta; //GeV2
//     double kappa2=z*(1-z)*MX2-mf*mf; //GeV2
//     double kappa=sqrt(kappa2); //GeV
//     double epsilon2=z*(1-z)*Q2+mf*mf;
//     double epsilon=sqrt(epsilon2); //GeV
//     double besselK1=TMath::BesselK1(epsilon*r/hbarc);
//     double besselJ1=TMath::BesselJ1(kappa*r/hbarc);
//     double dsigmadb2=0; //GeV0
//     if(mA==1)
//         dsigmadb2=mDipoleModel->dsigmadb2ep(r, b, xpom);
//     else
//         dsigmadb2=mDipoleModel->coherentDsigmadb2(r, b, xpom);

//     double result = r*besselK1*besselJ1*dsigmadb2; //fm
//     if (std::isnan(result) || std::isinf(result)) return 0.;
//     return result;
// }

// //===================================================
// //           QQG Calculations
// //===================================================

// double InclusiveDiffractiveCrossSectionsIntegrals::dsigmadbetadz_QQG(double beta, double Q2, double W2, double ztilde) {
//     double MX2 = Q2*(1-beta)/beta;
//     double xpom = Kinematics::xpomeron(0., Q2, W2, sqrt(MX2));
    
//     double Phi=Phi_qqg(xpom, ztilde, Q2);
    
//     double alpha_S=0.15;
//     double ef=quarkCharge[mQuarkIndex];
    
//     double betafactor=(1-beta/ztilde)*(1-beta/ztilde)+beta/ztilde*beta/ztilde;
//     double result=alpha_S*alpha_em/(2*M_PI*M_PI*Q2)*betafactor*ef*ef*Phi; //GeV-2
//     result*=hbarc2; //fm2
//     result *= 1e7; //nb
//     // cout<< " beta="<<beta<<", Q2="<<Q2<<", W2="<<W2<<", ztilde="<<ztilde<<", result="<<result<<endl;
//     if(std::isnan(result) || std::isinf(result)) return 0.;
//     return result; //nb
// }

// double InclusiveDiffractiveCrossSectionsIntegrals::Phi_qqg(double xpom, double z, double Q2){

//     if (gPhiQQGCache.filled &&
//         gPhiQQGCache.xpom == xpom &&
//         gPhiQQGCache.z    == z &&
//         gPhiQQGCache.Q2   == Q2) {
//         return gPhiQQGCache.value;
//     }

//     // Set the state uiPhi_qqg()/ComputeAmplitudeQQGCore() will read
//     // (replaces Ampqqg->SetParameter(0, xpom); Ampqqg->SetParameter(1, z);).
//     mAmpQQGXpom = xpom;
//     mAmpQQGZ    = z;

//     if(mA>1){
//         LookupTableCache& lc = gLookupTableCache[this];
//         if(!(lc.valid && lc.lastXpom==xpom)){
//             dipoleModel()->createSigma_ep_LookupTable(xpom);
//             lc.lastXpom = xpom;
//             lc.valid = true;
//         }
//     }

//     mQ2=Q2;

//     int nregions, neval, fail;
//     double integral[1], error[1], prob[1];

//     // Cuhre works on the unit cube [0,1]^2; uiPhi_qqg maps x[0],x[1] back
//     // to physical (b, k2) and applies the Jacobian internally.
//     // cubacores(0, 0);
//     Cuhre(2, 1, Cuhre_Integrand_QQG, this, 1, 1e-5, 1e-12, 0, 1000, 500000, 9,
//           NULL, NULL, &nregions, &neval, &fail, integral, error, prob);

//     double result = integral[0]/hbarc; //GeV0
//     if(fail != 0 || std::isnan(result) || std::isinf(result)) result = 0.;

//     gPhiQQGCache.xpom = xpom;
//     gPhiQQGCache.z    = z;
//     gPhiQQGCache.Q2   = Q2;
//     gPhiQQGCache.value = result;
//     gPhiQQGCache.filled = true;

//     return result;
// }

// double InclusiveDiffractiveCrossSectionsIntegrals::uiPhi_qqg(const double* var) {
//     // Map Cuba's x[0], x[1] in [0,1] to physical bounds (b, k2)
//     double bRange = 3. * dipoleModel()->nucleus()->radius();
//     double b  = var[0] * bRange;
//     double k2 = 1e-4 + var[1] * (mQ2 - 1e-4);
//     double jacobian = bRange * (mQ2 - 1e-4);

//     double Q2 = mQ2;

//     // Pure-C++ integrand, no TF1/WrappedTF1 in the loop.
//     AmpQQG_Integrand integrand{this, mAmpQQGXpom, mAmpQQGZ, sqrt(k2), b};
//     ROOT::Math::Functor1D functor(integrand);
//     GIAmpqqg.SetFunction(functor);

//     double rlow=1e-3;
//     double rhigh=100;
//     double amp_qqg=GIAmpqqg.Integral(rlow,rhigh); //fm2
//     amp_qqg/=hbarc2; //GeV-2

//     double result = (2*M_PI)*b/hbarc*k2*k2*log(Q2/k2)*amp_qqg*amp_qqg; //GeV-1
//     result *= jacobian; // Jacobian for the [0,1]^2 -> (b,k2) change of variables

//     if (std::isnan(result) || std::isinf(result)) return 0.;
//     return result; //GeV-1
// }

// int InclusiveDiffractiveCrossSectionsIntegrals::Cuhre_Integrand_QQG(
//         const int *ndim, const double x[], const int *ncomp, double f[], void *userdata) {
//     auto* self = static_cast<InclusiveDiffractiveCrossSectionsIntegrals*>(userdata);
//     f[0] = self->uiPhi_qqg(x);
//     return 0;
// }

// double InclusiveDiffractiveCrossSectionsIntegrals::ComputeAmplitudeQQGCore(
//         double r, double xpom, double z, double k, double b) const {
//     if(r>100.) return 0; //Gauss nodes/IntegralUp sometimes probe values that break the dipole model
//     double besselK2=ROOT::Math::cyl_bessel_k(2, sqrt(z)*k*r/hbarc);
//     double besselJ2=ROOT::Math::cyl_bessel_j(2, sqrt(1-z)*k*r/hbarc);
//     double dsigmadb2=0; //GeV0
//     if(mA==1)
//         dsigmadb2=mDipoleModel->dsigmadb2ep(r, b, xpom);
//     else
//         dsigmadb2=mDipoleModel->coherentDsigmadb2(r, b, xpom);
//     double term=1-0.5*dsigmadb2;
//     double dsigmadb2tilde=2*(1-term*term);
//     double result=r*besselK2*besselJ2*dsigmadb2tilde; //fm
//     if (std::isnan(result) || std::isinf(result)) return 0.;
//     return result;
// }

// // ---------------------------------------------------------------------------
// // DEAD CODE: InclusiveDiffractiveCrossSections::uiAmplitude_0/1/qqg below are
// // no longer called anywhere in this file now that Amplitude_0/Amplitude_1/
// // uiPhi_qqg use the pure-C++ ComputeAmplitude0Core/ComputeAmplitude1Core/
// // ComputeAmplitudeQQGCore functions above instead of TF1. Left in place
// // because their declarations live in InclusiveDiffractiveCrossSections.h,
// // which wasn't available to check for other callers/derived classes. Once
// // you confirm nothing else uses them, delete these three definitions and
// // their declarations in the base header.
// // ---------------------------------------------------------------------------

// double InclusiveDiffractiveCrossSections::uiAmplitude_0(double *var, double *par){
//     double r=*var;
//     if(r>100.) return 0; //IntegralUp sometimes tries too large values that break the dipole model
//     double beta=par[0];
//     double xpom=par[1];
//     double z=par[2];
//     double Q2=par[3];
//     double b=par[4];
//     double mf=par[5];
//     double MX2=Q2*(1-beta)/beta;
//     double kappa2=z*(1-z)*MX2-mf*mf;
//     double kappa=sqrt(kappa2);
//     double epsilon2=z*(1-z)*Q2+mf*mf;
//     double epsilon=sqrt(epsilon2); //GeV
//     double besselK0=TMath::BesselK0(epsilon*r/hbarc);
//     double besselJ0=TMath::BesselJ0(kappa*r/hbarc);
//     double dsigmadb2=0;
//     if(mA==1)
//         dsigmadb2=mDipoleModel->dsigmadb2ep(r, b, xpom);
//     else{
//         dsigmadb2=mDipoleModel->coherentDsigmadb2(r, b, xpom);
//     }
//     double result = r*besselK0*besselJ0*dsigmadb2; //fm
//     if (std::isnan(result) || std::isinf(result)) return 0.;
//     return result;
// }

// double InclusiveDiffractiveCrossSections::uiAmplitude_1(double *var, double *par){
//     double r=*var; //fm
//     if(r>100.) return 0; //IntegralUp sometimes tries too large values that break the dipole model
//     double beta=par[0];
//     double xpom=par[1];
//     double z=par[2];
//     double Q2=par[3]; //GeV2
//     double b=par[4];
//     double mf=par[5];
//     double MX2=Q2*(1-beta)/beta; //GeV2
//     double kappa2=z*(1-z)*MX2-mf*mf; //GeV2
//     double kappa=sqrt(kappa2); //GeV
//     double epsilon2=z*(1-z)*Q2+mf*mf;
//     double epsilon=sqrt(epsilon2); //GeV
//     double besselK1=TMath::BesselK1(epsilon*r/hbarc);
//     double besselJ1=TMath::BesselJ1(kappa*r/hbarc);
//     double dsigmadb2=0; //GeV0
//     if(mA==1)
//         dsigmadb2=mDipoleModel->dsigmadb2ep(r, b, xpom);
//     else
//         dsigmadb2=mDipoleModel->coherentDsigmadb2(r, b, xpom);

//     double result = r*besselK1*besselJ1*dsigmadb2; //fm
//     if (std::isnan(result) || std::isinf(result)) return 0.;
//     return result;
// }

// double InclusiveDiffractiveCrossSections::uiAmplitude_qqg(double *var, double *par){
//     double r=*var; //fm
//     if(r>100.) return 0; //IntegralUp sometimes tries too large values that break the dipole model
//     double xpom=par[0];
//     double z=par[1];
//     double k=par[2]; //GeV
//     double b=par[3];
    
//     double besselK2=ROOT::Math::cyl_bessel_k(2, sqrt(z)*k*r/hbarc);
//     double besselJ2=ROOT::Math::cyl_bessel_j(2, sqrt(1-z)*k*r/hbarc);
//     double dsigmadb2=0; //GeV0
//     if(mA==1)
//         dsigmadb2=mDipoleModel->dsigmadb2ep(r, b, xpom);
//     else
//         dsigmadb2=mDipoleModel->coherentDsigmadb2(r, b, xpom);
//     double term=1-0.5*dsigmadb2;
//     double dsigmadb2tilde=2*(1-term*term);
//     double result=r*besselK2*besselJ2*dsigmadb2tilde; //fm
//     if (std::isnan(result) || std::isinf(result)) {
//         return 0.0;
//     }   
//     return result;
// }


////==========================================================================================
// bpn : new improved script
///==========================================================================================
//==============================================================================
//  InclusiveDiffractiveCrossSection.cpp
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
//  Author: Tobias Toll
//  Last update:
//  $Date: 2024-06-03 11:27:05 -0400 (Mon, 03 Jun 2024) $
//  $Author: ullrich $
//==============================================================================
// #include "surrogateCrossSection.h"
// #include <torch/script.h>
#include "Math/SpecFunc.h"
#include "TFile.h"
#include <iostream>
#include <cmath>
#include <algorithm>
#include <algorithm>
#include <limits>
#include "Constants.h"
#include "Nucleus.h"
#include "DipoleModel.h"
#include "AlphaStrong.h"
#include "Math/IntegratorMultiDim.h"
#include "Math/Functor.h"
#include "TMath.h"
#include "WaveOverlap.h"
#include "Kinematics.h"
//#include "TableGeneratorSettings.h"
#include "Settings.h"
#include "Enumerations.h"
#include "TF1.h"
#include "TH1F.h"
#include "cuba.h"
#include "InclusiveDiffractiveCrossSections.h"
#include "DglapEvolution.h"
#include "Math/WrappedTF1.h"
#include "Math/GaussIntegrator.h"
#include "DipoleModelParameters.h"
#include <unordered_map>


#include "Math/GaussLegendreIntegrator.h"
#include "Math/Functor.h"   // ROOT::Math::Functor1D

#define PR(x) cout << #x << " = " << (x) << endl;

//------------------------------------------------------------------------------
// Persistent integrator caches for Phi_0()/Phi_1().
//
// These two functions used to `new`/`delete` a TF1 + WrappedTF1 +
// GaussIntegrator triple EVERY time they were called. Since they're called
// once per quark species per kinematic point (i.e. potentially millions of
// times over a table-generation run), that repeated allocation/ROOT-object
// construction was the dominant cost, far outweighing the integrator
// tolerance settings. Caching by `this` keeps behavior correct even if more
// than one InclusiveDiffractiveCrossSectionsIntegrals instance exists.
// thread_local so this stays safe if instances are ever used from multiple
// threads.
//------------------------------------------------------------------------------
namespace {
    /*
    struct Phi0IntegratorCache {
        ROOT::Math::GaussIntegrator* gi = nullptr;

        // Memoization: dsigmadbetadz_T() and dsigmadbetadz_L() both call
        // Phi_0() with identical (beta, xpom, z, Q2, mf) for a given quark
        // species -- the longitudinal call was previously repeating the
        // transverse call's expensive double integral from scratch. One
        // slot per quark species (4) is enough to catch this since that's
        // the maximum "distance" between a T call and its matching L call.
        static constexpr int kCacheSize = 4;
        double keyBeta[kCacheSize] = {};
        double keyXpom[kCacheSize] = {};
        double keyZ[kCacheSize] = {};
        double keyQ2[kCacheSize] = {};
        double keyMf[kCacheSize] = {};
        double value[kCacheSize] = {};
        bool filled[kCacheSize] = {false, false, false, false};
        int nextSlot = 0;
    };
    struct Phi1IntegratorCache {
        ROOT::Math::GaussIntegrator* gi = nullptr;
        // Phi_1() is only ever called from dsigmadbetadz_T(), so it doesn't
        // get the T/L duplication Phi_0 does -- no memoization needed here.
    };
    */
   struct Phi0IntegratorCache {
       // Memoization: dsigmadbetadz_T() and dsigmadbetadz_L() both call
       // Phi_0() with identical (beta, xpom, z, Q2, mf) for a given quark
       // species -- the longitudinal call was previously repeating the
       // transverse call's expensive double integral from scratch. One
       // slot per quark species (4) is enough to catch this since that's
       // the maximum "distance" between a T call and its matching L call.
       static constexpr int kCacheSize = 4;
       double keyBeta[kCacheSize] = {};
       double keyXpom[kCacheSize] = {};
       double keyZ[kCacheSize] = {};
       double keyQ2[kCacheSize] = {};
       double keyMf[kCacheSize] = {};
       double value[kCacheSize] = {};
       bool filled[kCacheSize] = {false, false, false, false};
       int nextSlot = 0;
    };
// Phi1IntegratorCache is no longer needed at all -- Phi_1 had no memoization
// logic to begin with, so once `gi` moves to a class member there's nothing
// left for a per-instance cache struct to hold.

    // Pure C++ outer-integral integrands (no TF1/WrappedTF1), mirroring the
    // Amp0_Integrand/Amp1_Integrand/AmpQQG_Integrand pattern used for the
    // inner r-integrals. operator()(b) reproduces uiPhi_0/uiPhi_1 exactly.
    /*
    struct Phi0_Integrand {
        InclusiveDiffractiveCrossSectionsIntegrals* self;
        double beta, xpom, z, Q2, mf;
        double operator()(double b) const {
            double amplitude = self->Amplitude_0(b); //fm2
            double result = 2*M_PI*b*amplitude*amplitude; //fm5
            if (std::isnan(result) || std::isinf(result)) return 0.;
            return result;
        }
    };

    struct Phi1_Integrand {
        InclusiveDiffractiveCrossSectionsIntegrals* self;
        double beta, xpom, z, Q2, mf;
        double operator()(double b) const {
            double amplitude = self->Amplitude_1(b); //fm2
            double result = 2*M_PI*b*amplitude*amplitude; //fm5
            if (std::isnan(result) || std::isinf(result)) return 0.;
            return result;
        }
    };
    */
   struct Phi0_Integrand {
        InclusiveDiffractiveCrossSectionsIntegrals* self;
        double beta, xpom, z, Q2, mf;
        double operator()(double b) const {
            double amplitude = self->Amplitude_0(b, beta, xpom, z, Q2, mf); //fm2
            double result = 2*M_PI*b*amplitude*amplitude; //fm5
            if (std::isnan(result) || std::isinf(result)) return 0.;
            return result;
        }
    };
    // Phi1_Integrand analogous, calling self->Amplitude_1(b, beta, xpom, z, Q2, mf)
    struct Phi1_Integrand {
        InclusiveDiffractiveCrossSectionsIntegrals* self;
        double beta, xpom, z, Q2, mf;
        double operator()(double b) const {
            double amplitude = self->Amplitude_1(b, beta, xpom, z, Q2, mf); //fm2
            double result = 2*M_PI*b*amplitude*amplitude; //fm5
            if (std::isnan(result) || std::isinf(result)) return 0.;
            return result;
        }
    };
    // Phi_0/Phi_1/Phi_qqg all call dipoleModel()->createSigma_ep_LookupTable(xpom)
    // for nuclear targets (mA>1). Within one kinematic point, that's called
    // once per quark species (and once more for the longitudinal pass) with
    // the *same* xpom every time. Cache by (instance, xpom) so it's only
    // rebuilt when xpom actually changes.
    struct LookupTableCache {
        double lastXpom = std::numeric_limits<double>::quiet_NaN();
        bool valid = false;
    };


    // Phi_qqg(xpom, z, Q2) doesn't depend on quark species -- only the ef^2
    // prefactor and the Mqq2 phase-space cut in dsigdbetadQ2dW2dz_qqg do.
    // The 4-species loop there calls dsigmadbetadz_QQG with identical
    // (beta, Q2, W2, ztilde) back-to-back, so xpom/z/Q2 are bit-identical
    // across those calls -- safe to cache exactly, no tolerance needed.
    struct PhiQQGCache {
        double xpom = std::numeric_limits<double>::quiet_NaN();
        double z    = std::numeric_limits<double>::quiet_NaN();
        double Q2   = std::numeric_limits<double>::quiet_NaN();
        double value = 0.0;
        bool filled = false;
    };

    thread_local std::unordered_map<const void*, LookupTableCache> gLookupTableCache;
    thread_local PhiQQGCache gPhiQQGCache;
}

InclusiveDiffractiveCrossSections::InclusiveDiffractiveCrossSections(){
    mSettings = EventGeneratorSettings::instance();
    mRandom = mSettings->randomGenerator();
    mS = Kinematics::s(mSettings->eBeam(), mSettings->hBeam());
    // PR(mS)
    mPhotonFlux.setS(mS);
    mCheckKinematics = true;
    mCrossSectionRatioLT = 0;
    
    mQ2=0;
    
//    TableGeneratorSettings* settings = TableGeneratorSettings::instance();
    // if(mDipoleModel) delete mDipoleModel;
    
    mDipoleModelParameterSet = mSettings->dipoleModelParameterSet();
    DipoleModelType model=mSettings->dipoleModelType();
    if (model==bSat) {
        mDipoleModel = new DipoleModel_bSat(mSettings);
    }
    else if(model==bNonSat){
        mDipoleModel = new DipoleModel_bNonSat(mSettings);
    }
    else {
        cout << "Integrals::init(): Error, model not implemented: "<< model << endl;
        exit(1);
    }
    mA=mSettings->A();

}

InclusiveDiffractiveCrossSections::~InclusiveDiffractiveCrossSections(){}

double* InclusiveDiffractiveCrossSections::dsigdbetadQ2dWdz_T_total(){return mDsigdbetadQ2dWdz_T_total;}
double* InclusiveDiffractiveCrossSections::dsigdbetadQ2dWdz_L_total(){return mDsigdbetadQ2dWdz_L_total;}
double* InclusiveDiffractiveCrossSections::dsigdbetadQ2dWdz_T_qqg(){ return mDsigdbetadQ2dWdz_T_qqg;};

//These three functions are unique for the sub-classes:
double InclusiveDiffractiveCrossSections::dsigmadbetadz_T(double, double, double, double){ return 0; }
double InclusiveDiffractiveCrossSections::dsigmadbetadz_L(double, double, double, double){ return 0; }
double InclusiveDiffractiveCrossSections::dsigmadbetadz_QQG(double, double, double, double){return 0;}
void InclusiveDiffractiveCrossSections::setTableCollection(TableCollection*){};



void InclusiveDiffractiveCrossSections::setCheckKinematics(bool val) {mCheckKinematics = val;}

void InclusiveDiffractiveCrossSections::setFockState(FockState val){
    mFockState=val;
}

FockState InclusiveDiffractiveCrossSections::getFockState(){
    return mFockState;
}

double InclusiveDiffractiveCrossSections::operator()(double MX2, double Q2, double W2, double z){
    return dsigdMX2dQ2dW2dz_total_checked(MX2, Q2, W2, z);
}

double InclusiveDiffractiveCrossSections::operator()(const double* array){
    double result=0;
    if(mFockState==QQ or mFockState==ALL){
        result+=dsigdbetadQ2dW2dz_total_checked(array[0], array[1], array[2], array[3]);
    }
    else if(mFockState==QQG or
            mFockState==ALL){
        result+=dsigdbetadQ2dW2dz_total_qqg_checked(array[0], array[1], array[2], array[3]);
    }
    else{
        cout<<"InclusiveDiffractiveCrossSectionsIntegrals::operator(): Fockstate is invalid, stopping"<<endl;
        exit(0);
    }
    // cout<<"InclusiveDiffractiveCrossSectionsIntegrals::operator(): result="<<result<<endl;
    return result;
}

double InclusiveDiffractiveCrossSections::unuranPDF(const double* array){
    //
    // array is: beta, log(Q2), W2, z
    //
    double result = 0;
    double Q2 = exp(array[1]);
    result = dsigdbetadQ2dW2dz_total_checked(array[0], Q2, array[2], array[3]);
    result *= Q2;   // Jacobian
    return log(result);
}

double InclusiveDiffractiveCrossSections::dsigdMX2dQ2dW2dz_total_checked(double MX2, double Q2, double W2, double z){
    double beta=Q2/(Q2+MX2);
    double jacobian=beta*beta/Q2;
    double result= dsigdbetadQ2dW2dz_total_checked(beta, Q2, W2, z);
    return jacobian*result;
}

double InclusiveDiffractiveCrossSections::dsigdbetadQ2dW2dz_total_checked(double beta, double Q2, double W2, double z){
    double result = 0;
    //
    //  Check if in kinematically allowed region
    double MX2=Kinematics::MX2(Q2, beta, 0);
    if (mCheckKinematics && !Kinematics::valid(mS, beta, Q2, W2, z, sqrt(MX2), false, (mSettings->verboseLevel() > 2) )) {
        if (mSettings->verboseLevel() > 2)
            cout << "InclusiveDiffractiveCrossSectionsIntegrals::dsigdbetadQ2dW2dz_total_checked(): warning, beta=" << beta << ", Q2=" << Q2 << ", W=" << sqrt(W2) << ", z="<< z
                << " is outside of kinematically allowed region. Return 0." << endl;
        return 0;
    }
    
    double csT = dsigdbetadQ2dW2dz_total(beta, Q2, W2, z, transverse);
    double csL = dsigdbetadQ2dW2dz_total(beta, Q2, W2, z, longitudinal);
    result = csT + csL;
    mCrossSectionRatioLT=csL/csT;
    //
    //  Polarization
    //
    if (mRandom->Uniform(result) <= csT)
        mPolarization = transverse;
    else
        mPolarization = longitudinal;
    //
    //  Quark Species
    //
    //
    
    if(mPolarization == transverse){
        bool isChosen=false;
        double cs_rejected=0;
        for(int i=0; i<4; i++){
            double R=mRandom->Uniform(csT-cs_rejected);
            double csTi=mDsigdbetadQ2dWdz_T_total[i];
            if(R <= csTi){
                mQuarkSpecies=i;
                isChosen=true;
                break;
            }
            cs_rejected+=mDsigdbetadQ2dWdz_T_total[i];
        }
        if(!isChosen) mQuarkSpecies=4;
    }
    else{
        bool isChosen=false;
        double cs_rejected=0;
        for(int i=0; i<4; i++){
            double R=mRandom->Uniform(csL-cs_rejected);
            double csLi=mDsigdbetadQ2dWdz_L_total[i];
            if(R <= csLi){
                mQuarkSpecies=i;
                isChosen=true;
                break;
            }
            cs_rejected+=mDsigdbetadQ2dWdz_L_total[i];
        }
        if(!isChosen) mQuarkSpecies=4;
    }
    
    // Print-out at high verbose levels
    //
    if (mSettings->verboseLevel() > 10) {     // Spinal Tap ;-)
        cout << "InclusiveDiffractiveCrossSectionsIntegrals::dsigdbetadQ2dW2dz_total_checked(): " << result;
        cout << " at beta=" << beta << ", Q2=" << Q2 << ", W=" << sqrt(W2) << ", z=" << z;
        cout << " (" << (mPolarization == transverse ? "transverse" : "longitudinal");
        cout << ')' << endl;
    }
    //
    // Check validity of return value
    //
    if (std::isnan(result)) {
        cout << "InclusiveDiffractiveCrossSectionsIntegrals::dsigdbetadQ2dW2dz_total_checked(): Error, return value = NaN at" << endl;
        cout << "                                            beta=" << beta << ", Q2=" << Q2 << ", W=" << sqrt(W2) << ", z=" << z << endl;
        result = 0;
    }
    if (std::isinf(result)) {
        cout << "InclusiveDiffractiveCrossSectionsIntegrals::dsigdbetadQ2dW2dz_total_checked(): Error, return value = inf at" << endl;
        cout << "                                            beta=" << beta << ", Q2=" << Q2 << ", W=" << sqrt(W2) << ", z=" << z << endl;
        result = 0;
    }
    if (result < 0) {
        cout << "InclusiveDiffractiveCrossSectionsIntegrals::dsigdbetadQ2dW2dz_total_checked(): Error, negative cross-section at" << endl;
        cout << "                                            beta=" << beta << ", Q2=" << Q2 << ", W=" << sqrt(W2) << ", z=" << endl;
        result = 0;
    }
    mTotalCS=result;
    return result; //nb/GeV4
}

double InclusiveDiffractiveCrossSections::dsigdbetadQ2dW2dz_total(double beta, double Q2, double W2, double z, GammaPolarization pol) {
    double result = 0;
    for(int i=0; i<4; i++){
    // for(int i=0; i<3; i++){
    // int i = 1;
        setQuarkIndex(i);
        double tmpresult = dsigdbetadz_total(beta, Q2, W2, z, pol); //nb
        if (mSettings->applyPhotonFlux()) tmpresult *= mPhotonFlux(Q2,W2,pol); //nb/GeV4
        if(pol == transverse)
            mDsigdbetadQ2dWdz_T_total[i]=tmpresult;
        if(pol == longitudinal)
            mDsigdbetadQ2dWdz_L_total[i]=tmpresult;
        result+=tmpresult;
    }
    return result; //nb/GeV4
}

double InclusiveDiffractiveCrossSections::dsigdbetadz_total(double beta, double Q2, double W2, double z, GammaPolarization pol) {
    double result = 0;
    if(pol==transverse){
        result = dsigmadbetadz_T(beta, Q2, W2, z);
    }
    else if(pol==longitudinal){
        result = dsigmadbetadz_L(beta, Q2, W2, z);
    }
    return result; //nb
}


double InclusiveDiffractiveCrossSections::crossSectionOfLastCall(){
    return mTotalCS;
}

GammaPolarization InclusiveDiffractiveCrossSections::polarizationOfLastCall(){
    return mPolarization;
}

unsigned int InclusiveDiffractiveCrossSections::quarkSpeciesOfLastCall(){
    return mQuarkSpecies;
}

double InclusiveDiffractiveCrossSections::quarkMassOfLastCall(){
    return Settings::quarkMass(mQuarkSpecies);
}

void InclusiveDiffractiveCrossSections::setQuarkIndex(unsigned int val){mQuarkIndex=val;}

DipoleModel* InclusiveDiffractiveCrossSections::dipoleModel(){
    return mDipoleModel;
}

double InclusiveDiffractiveCrossSections::crossSectionRatioLTOfLastCall() const {return mCrossSectionRatioLT;}


//===================================================
//           QQG Calculations
//===================================================
double InclusiveDiffractiveCrossSections::unuranPDF_qqg(const double* array){
    //
    // array is: beta, log(Q2), W2, z
    //
    double result = 0;
    double Q2 = exp(array[1]);

    double z=array[3];
    result = dsigdbetadQ2dW2dz_total_qqg_checked(array[0], Q2, array[2], z);
    result *= Q2;   // Jacobian log(Q2)->Q2
    return log(result);
}

double InclusiveDiffractiveCrossSections::dsigdbetadQ2dW2dz_total_qqg_checked(double beta, double Q2, double W2, double z){
    //
    //  Check if in kinematically allowed region
    //
    double MX2=Kinematics::MX2(Q2, beta, 0);
    if(mCheckKinematics && z<beta) {
        if (mSettings->verboseLevel() > 2)
            cout<<"z<beta, beta="<<beta<<", z="<<z<<endl;
        return 0; // For QQG beta < z < 1
    }
    if (mCheckKinematics && !Kinematics::valid(mS, beta, Q2, W2, z, sqrt(MX2), false, (mSettings->verboseLevel() > 2) )) {
        if (mSettings->verboseLevel() > 2)
            cout << "InclusiveDiffractiveCrossSectionsIntegrals::dsigdbetadQ2dW2dz_total_qqg_checked(): warning, beta=" << beta << ", Q2=" << Q2 << ", W=" << sqrt(W2) << ", z="<< z
                << " is outside of kinematically allowed region. Return 0." << endl;
        return 0;
    }
    double result = dsigdbetadQ2dW2dz_qqg(beta, Q2, W2, z);
    //
    //  Quark Species
    //
    //
    
    bool isChosen=false;
    double cs_rejected=0;
    for(int i=0; i<4; i++){
        double R=mRandom->Uniform(result-cs_rejected);
        double csTi=mDsigdbetadQ2dWdz_T_qqg[i];
        if(R <= csTi){
            mQuarkSpecies=i;
            isChosen=true;
            break;
        }
        cs_rejected+=mDsigdbetadQ2dWdz_T_qqg[i];
    }
    if(!isChosen) mQuarkSpecies=4;
    
    // Print-out at high verbose levels
    //
    if (mSettings->verboseLevel() > 10) {     // Spinal Tap ;-)
        cout << "InclusiveDiffractiveCrossSectionsIntegrals::dsigdbetadQ2dW2dz_total_qqg_checked(): " << result;
        cout << " at beta=" << beta << ", Q2=" << Q2 << ", W=" << sqrt(W2) << ", z=" << z<< endl;
    }
    //
    // Check validity of return value
    //
    if (std::isnan(result)) {
        cout << "InclusiveDiffractiveCrossSectionsIntegrals::dsigdbetadQ2dW2dz_total_qqg_checked(): Error, return value = NaN at" << endl;
        cout << "                                            beta=" << beta << ", Q2=" << Q2 << ", W=" << sqrt(W2) << ", z=" << z << endl;
        result = 0;
    }
    if (std::isinf(result)) {
        cout << "InclusiveDiffractiveCrossSectionsIntegrals::dsigdbetadQ2dW2dz_total_qqg_checked(): Error, return value = inf at" << endl;
        cout << "                                            beta=" << beta << ", Q2=" << Q2 << ", W=" << sqrt(W2) << ", z=" << z << endl;
        result = 0;
    }
    if (result < 0) {
        cout << "InclusiveDiffractiveCrossSectionsIntegrals::dsigdbetadQ2dW2dz_total_qqg_checked(): Error, negative cross-section at" << endl;
        cout << "                                            beta=" << beta << ", Q2=" << Q2 << ", W=" << sqrt(W2) << ", z=" << endl;
        result = 0;
    }
    mTotalCS_qqg=result;
    return result; //nb/GeV4
}

double InclusiveDiffractiveCrossSections::dsigdbetadQ2dW2dz_qqg(double beta, double Q2, double W2, double z) {
    double result = 0;
    for(int i=0; i<4; i++){
    // for(int i=0; i<3; i++){
    // int i = 1;
        setQuarkIndex(i);
        double mf = Settings::quarkMass(mQuarkIndex);
        double Mqq2=(z/beta-1)*Q2;
        if(Mqq2<4*mf*mf) continue; //Not enough phase-space for the q and qbar
        double tmpresult = dsigmadbetadz_QQG(beta, Q2, W2, z); //nb
        if (mSettings->applyPhotonFlux()) tmpresult *= mPhotonFlux(Q2,W2,transverse); //nb/GeV4
        mDsigdbetadQ2dWdz_T_qqg[i]=tmpresult;
        // cout << "beta=" << beta << ", Q2=" << Q2 << ", W=" << sqrt(W2) << ", z=" << z << ", quark species=" << i << ", cross-section=" << tmpresult << " nb" << endl;
        result+=tmpresult;
    }
    return result; //nb/GeV4 (with flux) or nb (without)
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  InclusiveDiffractiveCrossSectionsIntegrals:
//  Inheriting class from InclusiveDiffractiveCrossSections
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
InclusiveDiffractiveCrossSectionsIntegrals::InclusiveDiffractiveCrossSectionsIntegrals()
{
    //Set up the integrals
    //Amplitude0:
    if(Amp0) delete Amp0;
    Amp0=new TF1("Amp0", this, &InclusiveDiffractiveCrossSections::uiAmplitude_0, 0, 30., 5);
    
    if(WFAmp0) delete WFAmp0;
    WFAmp0=new ROOT::Math::WrappedTF1(*Amp0);
    
    GIAmp0.SetFunction(*WFAmp0);
    GIAmp0.SetRelTolerance(1e-8);
    GIAmp0.SetAbsTolerance(1e-15);
    

    //Amplitude1:
    if(Amp1) delete Amp1;
    Amp1=new TF1("Amp1", this, &InclusiveDiffractiveCrossSections::uiAmplitude_1, 0, 30. , 5);
    
    if(WFAmp1) delete WFAmp1;
    WFAmp1=new ROOT::Math::WrappedTF1(*Amp1);
    
    GIAmp1.SetFunction(*WFAmp1);   // BUGFIX: this was previously commented out, so GIAmp1
                                    // (used by Amplitude_1()/Phi_1(), part of the transverse
                                    // cross section) never had a valid integrand set.
    GIAmp1.SetRelTolerance(1e-8);
    GIAmp1.SetAbsTolerance(1e-15);
    //AmplitudeQQG:
    if(Ampqqg) delete Ampqqg;
    Ampqqg=new TF1("Ampqqg", this, &InclusiveDiffractiveCrossSections::uiAmplitude_qqg, 0, 30., 4);
    
    // if(WFAmpqqg) delete WFAmpqqg;
    WFAmpqqg=new ROOT::Math::WrappedTF1(*Ampqqg);
    
    GIAmpqqg.SetFunction(*WFAmpqqg);
    GIAmpqqg.SetRelTolerance(1e-8);
    GIAmpqqg.SetAbsTolerance(1e-15);

}

InclusiveDiffractiveCrossSectionsIntegrals::~InclusiveDiffractiveCrossSectionsIntegrals(){
    if(WFAmp0) delete WFAmp0;
    if(Amp0) delete Amp0;
    if(WFAmp1) delete WFAmp1;
    if(Amp1) delete Amp1;
    if(Ampqqg) delete Ampqqg;
}

double InclusiveDiffractiveCrossSectionsIntegrals::dsigmadbetadz_T(double beta, double Q2, double W2, double z) {
    double MX2 = Q2*(1-beta)/beta;
    double xpom = Kinematics::xpomeron(0., Q2, W2, sqrt(MX2));
    double mf = Settings::quarkMass(mQuarkIndex);
    double sqrtArg = 1.-4.*mf*mf/MX2;
    if (sqrtArg<0){
        if (mSettings->verboseLevel() > 2)
            cout<<"There is no phase-space for z, return 0."<<endl;
        return 0;
    }
    double z0 = (1.-sqrt(sqrtArg))/2.;
    if (z<z0 or z>1-z0){
        if (mSettings->verboseLevel() > 2)
            cout<<"z="<<z<<" which is smaller than z0="<<z0<<"or larger than (1-z0)="<<1-z0<<", return 0."<<endl;
        return 0;
    }
    double pt2 = z*(1-z)*Q2-mf*mf;
    if (pt2<0){
        if (mSettings->verboseLevel() > 2)
            cout<<"Not enough phase-space for quark transverse momenta, return 0. mf="<<mf<<endl;
        return 0;
    }
    if(z*(1-z)*MX2-mf*mf<0){ //kappa2<0
        if (mSettings->verboseLevel() > 2)
            cout<<"Not enough phase-space for quark production, return 0."<<endl;
        return 0;
    }
    double epsilon2=z*(1-z)*Q2+mf*mf;
    double ef=quarkCharge[mQuarkIndex];

    double Phi0=Phi_0(beta, xpom, z, Q2, mf);
    double Phi1=Phi_1(beta, xpom, z, Q2, mf);

    double prefactor=Nc*Q2*alpha_em/(4.*M_PI*beta*beta)*ef*ef*z*(1-z); //GeV2
    prefactor/=2.; //expanded z-range
    double term1=epsilon2*(z*z+(1-z)*(1-z))*Phi1; //GeV2*fm6
    double term2=mf*mf*Phi0; //GeV2*fm6
    
    double result=prefactor*(term1+term2); //GeV4*fm6
    result/=hbarc2*hbarc2; //fm2
    result *= 1e7; //nb
    return result;//nb
}

double InclusiveDiffractiveCrossSectionsIntegrals::dsigmadbetadz_L(double beta, double Q2, double W2, double z){
    
    double MX2 = Q2*(1-beta)/beta;
    double xpom = Kinematics::xpomeron(0., Q2, W2, sqrt(MX2));
    double mf =  Settings::quarkMass(mQuarkIndex);
    double pt2 = z*(1-z)*Q2-mf*mf;
    if (pt2<0){
        if (mSettings->verboseLevel() > 2)
            cout<<"Not enough phase-space for quark transverse momenta, return 0. mf="<<mf<<endl;
        return 0;
    }
    double sqrtArg=1.-4.*mf*mf/MX2;
    if (sqrtArg<0){
        if (mSettings->verboseLevel() > 2)
            cout<<"There is no phase-space for z, return 0."<<endl;
        return 0;
    }
    double z0=(1.-sqrt(sqrtArg))/2.;
    if (z<z0 or z>1-z0){
        if (mSettings->verboseLevel() > 2)
            cout<<"z="<<z<<" which is smaller than z0="<<z0<<"or larger than (1-z0)="<<1-z0<<", return 0."<<endl;
        return 0;
    }
    if(z*(1-z)*MX2-mf*mf<0){ //kappa2<0
        if (mSettings->verboseLevel() > 2)
            cout<<"Not enough phase-space for quark production, return 0."<<endl;
        return 0;
    }
    double ef=quarkCharge[mQuarkIndex];
    double Phi0=Phi_0(beta, xpom, z, Q2, mf); //fm6
    
    double term=Nc*Q2*Q2*alpha_em/(M_PI*beta*beta); //GeV4
    term*=ef*ef*z*z*z*(1-z)*(1-z)*(1-z);
    term*=Phi0; //fm6*GeV4
    term/=2.; //expanded z-range
    double result=term;
    result/=hbarc2*hbarc2; //fm2
    result *= 1e7; //nb
    return result; //nb
}

double InclusiveDiffractiveCrossSectionsIntegrals::Phi_0(double beta, double xpom, double z, double Q2, double mf){
    // Build (once) and reuse the TF1/WrappedTF1/GaussIntegrator instead of
    // allocating them on every call -- see Phi0IntegratorCache comment above.
    thread_local std::unordered_map<const void*, Phi0IntegratorCache> caches;
    Phi0IntegratorCache& c = caches[this];
    if (!c.fn) {
        c.fn = new TF1("Phi0", this, &InclusiveDiffractiveCrossSectionsIntegrals::uiPhi_0, 0, 30., 0);
        c.wf = new ROOT::Math::WrappedTF1(*c.fn);
        c.gi = new ROOT::Math::GaussIntegrator();
        c.gi->SetFunction(*c.wf);
        c.gi->SetRelTolerance(1e-6);
        c.gi->SetAbsTolerance(1e-12);
    }

    // Memoization: the longitudinal pass calls Phi_0 with the same inputs
    // the transverse pass already used for this quark species -- skip
    // recomputing the whole double integral if we've already got it.
    for (int i = 0; i < Phi0IntegratorCache::kCacheSize; ++i) {
        if (c.filled[i] && c.keyBeta[i]==beta && c.keyXpom[i]==xpom &&
            c.keyZ[i]==z && c.keyQ2[i]==Q2 && c.keyMf[i]==mf) {
            return c.value[i];
        }
    }

    Amp0->SetParameter(0, beta);
    Amp0->SetParameter(1, xpom);
    Amp0->SetParameter(2, z);
    Amp0->SetParameter(3, Q2);
    Amp0->SetParameter(5, mf);

    if(mA>1){
        LookupTableCache& lc = gLookupTableCache[this];
        if(!(lc.valid && lc.lastXpom==xpom)){
            dipoleModel()->createSigma_ep_LookupTable(xpom);
            lc.lastXpom = xpom;
            lc.valid = true;
        }
    }

    double blow=0;
    double result=c.gi->IntegralUp(blow); //fm6

    int slot = c.nextSlot;
    c.keyBeta[slot]=beta; c.keyXpom[slot]=xpom; c.keyZ[slot]=z; c.keyQ2[slot]=Q2; c.keyMf[slot]=mf;
    c.value[slot]=result; c.filled[slot]=true;
    c.nextSlot = (slot+1) % Phi0IntegratorCache::kCacheSize;

    return result;
}

double InclusiveDiffractiveCrossSectionsIntegrals::Phi_1(double beta, double xpom, double z, double Q2, double mf){
    // Build (once) and reuse the TF1/WrappedTF1/GaussIntegrator instead of
    // allocating them on every call -- see Phi1IntegratorCache comment above.
    thread_local std::unordered_map<const void*, Phi1IntegratorCache> caches;
    Phi1IntegratorCache& c = caches[this];
    if (!c.fn) {
        c.fn = new TF1("Phi1", this, &InclusiveDiffractiveCrossSectionsIntegrals::uiPhi_1, 0, 30., 0);
        c.wf = new ROOT::Math::WrappedTF1(*c.fn);
        c.gi = new ROOT::Math::GaussIntegrator();
        c.gi->SetFunction(*c.wf);
        c.gi->SetRelTolerance(1e-6);
        c.gi->SetAbsTolerance(1e-12);
    }

    Amp1->SetParameter(0, beta);
    Amp1->SetParameter(1, xpom);
    Amp1->SetParameter(2, z);
    Amp1->SetParameter(3, Q2);
    Amp1->SetParameter(5, mf);

    if(mA>1){
        LookupTableCache& lc = gLookupTableCache[this];
        if(!(lc.valid && lc.lastXpom==xpom)){
            dipoleModel()->createSigma_ep_LookupTable(xpom);
            lc.lastXpom = xpom;
            lc.valid = true;
        }
    }

    double blow=0;
    double result=c.gi->IntegralUp(blow); //fm6
    return result;
}

double InclusiveDiffractiveCrossSectionsIntegrals::uiPhi_0(double *var, double*){
    double b=*var;
    
    double amplitude=Amplitude_0(b); //fm2

    double result=2*M_PI*b*amplitude*amplitude; //fm5
    if(std::isnan(result) || std::isinf(result)) return 0.;
    return result;
}

double InclusiveDiffractiveCrossSectionsIntegrals::uiPhi_1(double *var, double*){
    double b=*var; //fm

    double amplitude=Amplitude_1(b); //fm2
    double result=2*M_PI*b*amplitude*amplitude; //fm5
    if(std::isnan(result) || std::isinf(result)) return 0.;
    return result;
}

double InclusiveDiffractiveCrossSectionsIntegrals::Amplitude_0(double b){
    //Gauss Integrator defined in header file and set in constructor

    Amp0->SetParameter(4, b);
    
    double rlow=1e-3;
    double rhigh=100;
    // double result=GIAmp0.IntegralUp(rlow); //fm2
    double result=GIAmp0.Integral(rlow,rhigh); //fm2
    if(std::isnan(result) || std::isinf(result)) return 0.;
    return result;
}


double InclusiveDiffractiveCrossSectionsIntegrals::Amplitude_1(double b){
    //Gauss Integrator defined in header file and set in constructor
    Amp1->SetParameter(4, b);
    
    double rlow=1e-3;
    double rhigh=100;
    // double result=GIAmp1.IntegralUp(rlow); //fm2
    double result=GIAmp1.Integral(rlow,rhigh); //fm2
    if(std::isnan(result) || std::isinf(result)) return 0.;
    return result;
}

double InclusiveDiffractiveCrossSections::uiAmplitude_0(double *var, double *par){
    double r=*var;
    if(r>100.) return 0; //IntegralUp sometimes tries too large values that break the dipole model
    double beta=par[0];
    double xpom=par[1];
    double z=par[2];
    double Q2=par[3];
    double b=par[4];
    double mf=par[5];
    double MX2=Q2*(1-beta)/beta;
    double kappa2=z*(1-z)*MX2-mf*mf;
    double kappa=sqrt(kappa2);
    double epsilon2=z*(1-z)*Q2+mf*mf;
    double epsilon=sqrt(epsilon2); //GeV
    double besselK0=TMath::BesselK0(epsilon*r/hbarc);
    double besselJ0=TMath::BesselJ0(kappa*r/hbarc);
    double dsigmadb2=0;
    if(mA==1)
        dsigmadb2=mDipoleModel->dsigmadb2ep(r, b, xpom);
    else{
        dsigmadb2=mDipoleModel->coherentDsigmadb2(r, b, xpom);
    }
    double result = r*besselK0*besselJ0*dsigmadb2; //fm
    if (std::isnan(result) || std::isinf(result)) return 0.;
    return result;
}

double InclusiveDiffractiveCrossSections::uiAmplitude_1(double *var, double *par){
    double r=*var; //fm
    if(r>100.) return 0; //IntegralUp sometimes tries too large values that break the dipole model
    double beta=par[0];
    double xpom=par[1];
    double z=par[2];
    double Q2=par[3]; //GeV2
    double b=par[4];
    double mf=par[5];
    double MX2=Q2*(1-beta)/beta; //GeV2
    double kappa2=z*(1-z)*MX2-mf*mf; //GeV2
    double kappa=sqrt(kappa2); //GeV
    double epsilon2=z*(1-z)*Q2+mf*mf;
    double epsilon=sqrt(epsilon2); //GeV
    double besselK1=TMath::BesselK1(epsilon*r/hbarc);
    double besselJ1=TMath::BesselJ1(kappa*r/hbarc);
    double dsigmadb2=0; //GeV0
    if(mA==1)
        dsigmadb2=mDipoleModel->dsigmadb2ep(r, b, xpom);
    else
        dsigmadb2=mDipoleModel->coherentDsigmadb2(r, b, xpom);

    double result = r*besselK1*besselJ1*dsigmadb2; //fm
    if (std::isnan(result) || std::isinf(result)) return 0.;
    return result;
}

//===================================================
//           QQG Calculations
//===================================================

double InclusiveDiffractiveCrossSectionsIntegrals::dsigmadbetadz_QQG(double beta, double Q2, double W2, double ztilde) {
    double MX2 = Q2*(1-beta)/beta;
    double xpom = Kinematics::xpomeron(0., Q2, W2, sqrt(MX2));
    
    double Phi=Phi_qqg(xpom, ztilde, Q2);
    
    double alpha_S=0.15;
    double ef=quarkCharge[mQuarkIndex];
    
    double betafactor=(1-beta/ztilde)*(1-beta/ztilde)+beta/ztilde*beta/ztilde;
    double result=alpha_S*alpha_em/(2*M_PI*M_PI*Q2)*betafactor*ef*ef*Phi; //GeV-2
    result*=hbarc2; //fm2
    result *= 1e7; //nb
    // cout<< " beta="<<beta<<", Q2="<<Q2<<", W2="<<W2<<", ztilde="<<ztilde<<", result="<<result<<endl;
    if(std::isnan(result) || std::isinf(result)) return 0.;
    return result; //nb
}

double InclusiveDiffractiveCrossSectionsIntegrals::Phi_qqg(double xpom, double z, double Q2){

    if (gPhiQQGCache.filled &&
        gPhiQQGCache.xpom == xpom &&
        gPhiQQGCache.z    == z &&
        gPhiQQGCache.Q2   == Q2) {
        return gPhiQQGCache.value;
    }

    Ampqqg->SetParameter(0, xpom);
    Ampqqg->SetParameter(1, z);

    if(mA>1){
        LookupTableCache& lc = gLookupTableCache[this];
        if(!(lc.valid && lc.lastXpom==xpom)){
            dipoleModel()->createSigma_ep_LookupTable(xpom);
            lc.lastXpom = xpom;
            lc.valid = true;
        }
    }

    mQ2=Q2;

    int nregions, neval, fail;
    double integral[1], error[1], prob[1];

    // Cuhre works on the unit cube [0,1]^2; uiPhi_qqg maps x[0],x[1] back
    // to physical (b, k2) and applies the Jacobian internally.
    // cubacores(0, 0);
    Cuhre(2, 1, Cuhre_Integrand_QQG, this, 1, 1e-6, 1e-12, 0, 1000, 500000, 9,
          NULL, NULL, &nregions, &neval, &fail, integral, error, prob);

    double result = integral[0]/hbarc; //GeV0
    if(fail != 0 || std::isnan(result) || std::isinf(result)) result = 0.;

    gPhiQQGCache.xpom = xpom;
    gPhiQQGCache.z    = z;
    gPhiQQGCache.Q2   = Q2;
    gPhiQQGCache.value = result;
    gPhiQQGCache.filled = true;

    return result;
}

double InclusiveDiffractiveCrossSectionsIntegrals::uiPhi_qqg(const double* var) {
    // Map Cuba's x[0], x[1] in [0,1] to physical bounds (b, k2)
    double bRange = 3. * dipoleModel()->nucleus()->radius();
    double b  = var[0] * bRange;
    double k2 = 1e-4 + var[1] * (mQ2 - 1e-4);
    double jacobian = bRange * (mQ2 - 1e-4);

    double Q2 = mQ2;

    Ampqqg->SetParameter(2, sqrt(k2));
    Ampqqg->SetParameter(3, b);

    double rlow=1e-3;
    double rhigh=100;
    double amp_qqg=GIAmpqqg.Integral(rlow,rhigh); //fm2
    amp_qqg/=hbarc2; //GeV-2

    double result = (2*M_PI)*b/hbarc*k2*k2*log(Q2/k2)*amp_qqg*amp_qqg; //GeV-1
    result *= jacobian; // Jacobian for the [0,1]^2 -> (b,k2) change of variables

    if (std::isnan(result) || std::isinf(result)) return 0.;
    return result; //GeV-1
}

int InclusiveDiffractiveCrossSectionsIntegrals::Cuhre_Integrand_QQG(
        const int *ndim, const double x[], const int *ncomp, double f[], void *userdata) {
    auto* self = static_cast<InclusiveDiffractiveCrossSectionsIntegrals*>(userdata);
    f[0] = self->uiPhi_qqg(x);
    return 0;
}

double InclusiveDiffractiveCrossSections::uiAmplitude_qqg(double *var, double *par){
    double r=*var; //fm
    if(r>100.) return 0; //IntegralUp sometimes tries too large values that break the dipole model
    double xpom=par[0];
    double z=par[1];
    double k=par[2]; //GeV
    double b=par[3];
    
    double besselK2=ROOT::Math::cyl_bessel_k(2, sqrt(z)*k*r/hbarc);
    double besselJ2=ROOT::Math::cyl_bessel_j(2, sqrt(1-z)*k*r/hbarc);
    double dsigmadb2=0; //GeV0
    if(mA==1)
        dsigmadb2=mDipoleModel->dsigmadb2ep(r, b, xpom);
    else
        dsigmadb2=mDipoleModel->coherentDsigmadb2(r, b, xpom);
    double term=1-0.5*dsigmadb2;
    double dsigmadb2tilde=2*(1-term*term);
    double result=r*besselK2*besselJ2*dsigmadb2tilde; //fm
    if (std::isnan(result) || std::isinf(result)) {
        return 0.0;
    }   
    return result;
}
*/
 
//========================================================================================== 
// bpn : testing new integration method implementation
//==========================================================================================

namespace {

// Pure C++ functors (no TF1/WrappedTF1 inheritance) fed directly to the
// class-member GaussLegendreIntegrator via ROOT::Math::Functor1D.
struct Amp0_Integrand {
    InclusiveDiffractiveCrossSectionsIntegrals* self;
    double beta, xpom, z, Q2, mf, b;
    double operator()(double r) const {
        return self->ComputeAmplitude0Core(r, b, beta, xpom, z, Q2, mf);
    }
};

struct Amp1_Integrand {
    InclusiveDiffractiveCrossSectionsIntegrals* self;
    double beta, xpom, z, Q2, mf, b;
    double operator()(double r) const {
        return self->ComputeAmplitude1Core(r, b, beta, xpom, z, Q2, mf);
    }
};

struct AmpQQG_Integrand {
    InclusiveDiffractiveCrossSectionsIntegrals* self;
    double xpom, z, k, b;
    double operator()(double r) const {
        return self->ComputeAmplitudeQQGCore(r, xpom, z, k, b);
    }
};

} // namespace
/*
InclusiveDiffractiveCrossSectionsIntegrals::InclusiveDiffractiveCrossSectionsIntegrals()
{
    // GIAmp0/GIAmp1/GIAmpqqg are fixed-order (256-pt) Gauss-Legendre integrators
    // (default-constructed via in-class member initializers in the header).
    // No TF1/WrappedTF1 setup is needed anymore: the integrand functor is built
    // and handed to SetFunction() fresh on every call in Amplitude_0/Amplitude_1/
    // uiPhi_qqg below.
    
}*/
InclusiveDiffractiveCrossSectionsIntegrals::InclusiveDiffractiveCrossSectionsIntegrals()
{
    GIAmp0.SetRelTolerance(1e-7);
    GIAmp0.SetAbsTolerance(0.);

    GIAmp1.SetRelTolerance(1e-7);
    GIAmp1.SetAbsTolerance(0.);

    GIAmpqqg.SetRelTolerance(1e-7);
    GIAmpqqg.SetAbsTolerance(0.);

    GIPhi0.SetRelTolerance(1e-5);
    GIPhi0.SetAbsTolerance(1e-10);

    GIPhi1.SetRelTolerance(1e-5);
    GIPhi1.SetAbsTolerance(1e-10);
}

InclusiveDiffractiveCrossSectionsIntegrals::~InclusiveDiffractiveCrossSectionsIntegrals(){
}

double InclusiveDiffractiveCrossSectionsIntegrals::dsigmadbetadz_T(double beta, double Q2, double W2, double z) {
    double MX2 = Q2*(1-beta)/beta;
    double xpom = Kinematics::xpomeron(0., Q2, W2, sqrt(MX2));
    double mf = Settings::quarkMass(mQuarkIndex);
    double sqrtArg = 1.-4.*mf*mf/MX2;
    if (sqrtArg<0){
        if (mSettings->verboseLevel() > 2)
            cout<<"There is no phase-space for z, return 0."<<endl;
        return 0;
    }
    double z0 = (1.-sqrt(sqrtArg))/2.;
    if (z<z0 or z>1-z0){
        if (mSettings->verboseLevel() > 2)
            cout<<"z="<<z<<" which is smaller than z0="<<z0<<"or larger than (1-z0)="<<1-z0<<", return 0."<<endl;
        return 0;
    }
    double pt2 = z*(1-z)*Q2-mf*mf;
    if (pt2<0){
        if (mSettings->verboseLevel() > 2)
            cout<<"Not enough phase-space for quark transverse momenta, return 0. mf="<<mf<<endl;
        return 0;
    }
    if(z*(1-z)*MX2-mf*mf<0){ //kappa2<0
        if (mSettings->verboseLevel() > 2)
            cout<<"Not enough phase-space for quark production, return 0."<<endl;
        return 0;
    }
    double epsilon2=z*(1-z)*Q2+mf*mf;
    double ef=quarkCharge[mQuarkIndex];

    double Phi0=Phi_0(beta, xpom, z, Q2, mf);
    double Phi1=Phi_1(beta, xpom, z, Q2, mf);

    double prefactor=Nc*Q2*alpha_em/(4.*M_PI*beta*beta)*ef*ef*z*(1-z); //GeV2
    prefactor/=2.; //expanded z-range
    double term1=epsilon2*(z*z+(1-z)*(1-z))*Phi1; //GeV2*fm6
    double term2=mf*mf*Phi0; //GeV2*fm6
    
    double result=prefactor*(term1+term2); //GeV4*fm6
    result/=hbarc2*hbarc2; //fm2
    result *= 1e7; //nb
    return result;//nb
}

double InclusiveDiffractiveCrossSectionsIntegrals::dsigmadbetadz_L(double beta, double Q2, double W2, double z){
    
    double MX2 = Q2*(1-beta)/beta;
    double xpom = Kinematics::xpomeron(0., Q2, W2, sqrt(MX2));
    double mf =  Settings::quarkMass(mQuarkIndex);
    double pt2 = z*(1-z)*Q2-mf*mf;
    if (pt2<0){
        if (mSettings->verboseLevel() > 2)
            cout<<"Not enough phase-space for quark transverse momenta, return 0. mf="<<mf<<endl;
        return 0;
    }
    double sqrtArg=1.-4.*mf*mf/MX2;
    if (sqrtArg<0){
        if (mSettings->verboseLevel() > 2)
            cout<<"There is no phase-space for z, return 0."<<endl;
        return 0;
    }
    double z0=(1.-sqrt(sqrtArg))/2.;
    if (z<z0 or z>1-z0){
        if (mSettings->verboseLevel() > 2)
            cout<<"z="<<z<<" which is smaller than z0="<<z0<<"or larger than (1-z0)="<<1-z0<<", return 0."<<endl;
        return 0;
    }
    if(z*(1-z)*MX2-mf*mf<0){ //kappa2<0
        if (mSettings->verboseLevel() > 2)
            cout<<"Not enough phase-space for quark production, return 0."<<endl;
        return 0;
    }
    double ef=quarkCharge[mQuarkIndex];
    double Phi0=Phi_0(beta, xpom, z, Q2, mf); //fm6
    
    double term=Nc*Q2*Q2*alpha_em/(M_PI*beta*beta); //GeV4
    term*=ef*ef*z*z*z*(1-z)*(1-z)*(1-z);
    term*=Phi0; //fm6*GeV4
    term/=2.; //expanded z-range
    double result=term;
    result/=hbarc2*hbarc2; //fm2
    result *= 1e7; //nb
    return result; //nb
}
/*
double InclusiveDiffractiveCrossSectionsIntegrals::Phi_0(double beta, double xpom, double z, double Q2, double mf){
    // Build (once) and reuse the TF1/WrappedTF1/GaussIntegrator instead of
    // allocating them on every call -- see Phi0IntegratorCache comment above.
    // (This is the outer b-integral for Phi_0 itself; unrelated to the inner
    // r-integral refactor below, which only touches Amplitude_0/Amplitude_1/uiPhi_qqg.)
    thread_local std::unordered_map<const void*, Phi0IntegratorCache> caches;
    Phi0IntegratorCache& c = caches[this];
    if (!c.fn) {
        c.fn = new TF1("Phi0", this, &InclusiveDiffractiveCrossSectionsIntegrals::uiPhi_0, 0, 30., 0);
        c.wf = new ROOT::Math::WrappedTF1(*c.fn);
        c.gi = new ROOT::Math::GaussIntegrator();
        c.gi->SetFunction(*c.wf);
        c.gi->SetRelTolerance(1e-6);
        c.gi->SetAbsTolerance(1e-12);
    }

    // Memoization: the longitudinal pass calls Phi_0 with the same inputs
    // the transverse pass already used for this quark species -- skip
    // recomputing the whole double integral if we've already got it.
    for (int i = 0; i < Phi0IntegratorCache::kCacheSize; ++i) {
        if (c.filled[i] && c.keyBeta[i]==beta && c.keyXpom[i]==xpom &&
            c.keyZ[i]==z && c.keyQ2[i]==Q2 && c.keyMf[i]==mf) {
            return c.value[i];
        }
    }

    // Set the state Amplitude_0() will read when this outer b-integral
    // drives calls into it (replaces the old Amp0->SetParameter(0..3,5,...)).
    mAmp0Beta = beta;
    mAmp0Xpom = xpom;
    mAmp0Z    = z;
    mAmp0Q2   = Q2;
    mAmp0Mf   = mf;

    if(mA>1){
        LookupTableCache& lc = gLookupTableCache[this];
        if(!(lc.valid && lc.lastXpom==xpom)){
            dipoleModel()->createSigma_ep_LookupTable(xpom);
            lc.lastXpom = xpom;
            lc.valid = true;
        }
    }

    double blow=0;
    double result=c.gi->IntegralUp(blow); //fm6

    int slot = c.nextSlot;
    c.keyBeta[slot]=beta; c.keyXpom[slot]=xpom; c.keyZ[slot]=z; c.keyQ2[slot]=Q2; c.keyMf[slot]=mf;
    c.value[slot]=result; c.filled[slot]=true;
    c.nextSlot = (slot+1) % Phi0IntegratorCache::kCacheSize;

    return result;
}


double InclusiveDiffractiveCrossSectionsIntegrals::Phi_1(double beta, double xpom, double z, double Q2, double mf){
    // Build (once) and reuse the TF1/WrappedTF1/GaussIntegrator instead of
    // allocating them on every call -- see Phi1IntegratorCache comment above.
    thread_local std::unordered_map<const void*, Phi1IntegratorCache> caches;
    Phi1IntegratorCache& c = caches[this];
    if (!c.fn) {
        c.fn = new TF1("Phi1", this, &InclusiveDiffractiveCrossSectionsIntegrals::uiPhi_1, 0, 30., 0);
        c.wf = new ROOT::Math::WrappedTF1(*c.fn);
        c.gi = new ROOT::Math::GaussIntegrator();
        c.gi->SetFunction(*c.wf);
        c.gi->SetRelTolerance(1e-6);
        c.gi->SetAbsTolerance(1e-12);
    }

    // Set the state Amplitude_1() will read (replaces Amp1->SetParameter(0..3,5,...)).
    mAmp1Beta = beta;
    mAmp1Xpom = xpom;
    mAmp1Z    = z;
    mAmp1Q2   = Q2;
    mAmp1Mf   = mf;

    if(mA>1){
        LookupTableCache& lc = gLookupTableCache[this];
        if(!(lc.valid && lc.lastXpom==xpom)){
            dipoleModel()->createSigma_ep_LookupTable(xpom);
            lc.lastXpom = xpom;
            lc.valid = true;
        }
    }

    double blow=0;
    double result=c.gi->IntegralUp(blow); //fm6
    return result;
}
*/
double InclusiveDiffractiveCrossSectionsIntegrals::Phi_0(double beta, double xpom, double z, double Q2, double mf){
    // Memoization cache (values only) is still per-(thread, instance) since
    // multiple threads may run different instances -- GIPhi0 itself is now a
    // plain member, same treatment as GIAmp0/GIAmp1 for the inner r-integral.
    thread_local std::unordered_map<const void*, Phi0IntegratorCache> caches;
    Phi0IntegratorCache& c = caches[this];

    // Memoization: the longitudinal pass calls Phi_0 with the same inputs
    // the transverse pass already used for this quark species -- skip
    // recomputing the whole double integral if we've already got it.
    for (int i = 0; i < Phi0IntegratorCache::kCacheSize; ++i) {
        if (c.filled[i] && c.keyBeta[i]==beta && c.keyXpom[i]==xpom &&
            c.keyZ[i]==z && c.keyQ2[i]==Q2 && c.keyMf[i]==mf) {
            return c.value[i];
        }
    }

    if(mA>1)
        dipoleModel()->createSigma_ep_LookupTable(xpom);


    Phi0_Integrand integrand{this, beta, xpom, z, Q2, mf};
    ROOT::Math::Functor1D functor(integrand);
    GIPhi0.SetFunction(functor);

    double blow=0;
    double result=GIPhi0.IntegralUp(blow); //fm6

    int slot = c.nextSlot;
    c.keyBeta[slot]=beta; c.keyXpom[slot]=xpom; c.keyZ[slot]=z; c.keyQ2[slot]=Q2; c.keyMf[slot]=mf;
    c.value[slot]=result; c.filled[slot]=true;
    c.nextSlot = (slot+1) % Phi0IntegratorCache::kCacheSize;

    return result;
}

double InclusiveDiffractiveCrossSectionsIntegrals::Phi_1(double beta, double xpom, double z, double Q2, double mf){
    if(mA>1)
        dipoleModel()->createSigma_ep_LookupTable(xpom);
  

    Phi1_Integrand integrand{this, beta, xpom, z, Q2, mf};
    ROOT::Math::Functor1D functor(integrand);
    GIPhi1.SetFunction(functor);

    double blow=0;
    double result=GIPhi1.IntegralUp(blow); //fm6
    return result;
}
/*
double InclusiveDiffractiveCrossSectionsIntegrals::uiPhi_0(double *var, double*){
    double b=*var;
    
    double amplitude=Amplitude_0(b); //fm2

    double result=2*M_PI*b*amplitude*amplitude; //fm5
    if(std::isnan(result) || std::isinf(result)) return 0.;
    return result;
}

double InclusiveDiffractiveCrossSectionsIntegrals::uiPhi_1(double *var, double*){
    double b=*var; //fm

    double amplitude=Amplitude_1(b); //fm2
    double result=2*M_PI*b*amplitude*amplitude; //fm5
    if(std::isnan(result) || std::isinf(result)) return 0.;
    return result;
}

double InclusiveDiffractiveCrossSectionsIntegrals::Amplitude_0(double b){
    // Pure-C++ integrand, no TF1/WrappedTF1 in the loop.
    Amp0_Integrand integrand{this, mAmp0Beta, mAmp0Xpom, mAmp0Z, mAmp0Q2, mAmp0Mf, b};
    ROOT::Math::Functor1D functor(integrand);
    GIAmp0.SetFunction(functor);

    double rlow=1e-3;
    double rhigh=100;
    double result=GIAmp0.Integral(rlow,rhigh); //fm2
    if(std::isnan(result) || std::isinf(result)) return 0.;
    return result;
}


double InclusiveDiffractiveCrossSectionsIntegrals::Amplitude_1(double b){
    // Pure-C++ integrand, no TF1/WrappedTF1 in the loop.
    Amp1_Integrand integrand{this, mAmp1Beta, mAmp1Xpom, mAmp1Z, mAmp1Q2, mAmp1Mf, b};
    ROOT::Math::Functor1D functor(integrand);
    GIAmp1.SetFunction(functor);

    double rlow=1e-3;
    double rhigh=100;
    double result=GIAmp1.Integral(rlow,rhigh); //fm2
    if(std::isnan(result) || std::isinf(result)) return 0.;
    return result;
}
*/
double InclusiveDiffractiveCrossSectionsIntegrals::Amplitude_0(
        double b, double beta, double xpom, double z, double Q2, double mf){
    Amp0_Integrand integrand{this, beta, xpom, z, Q2, mf, b};
    ROOT::Math::Functor1D functor(integrand);
    GIAmp0.SetFunction(functor);
    GIAmp0.SetRelTolerance(1e-8);

    double rlow=1e-3;
    double rhigh=100;
    double result=GIAmp0.Integral(rlow,rhigh); //fm2
    if(std::isnan(result) || std::isinf(result)) return 0.;
    return result;
}

double InclusiveDiffractiveCrossSectionsIntegrals::Amplitude_1(
        double b, double beta, double xpom, double z, double Q2, double mf){
    Amp1_Integrand integrand{this, beta, xpom, z, Q2, mf, b};
    ROOT::Math::Functor1D functor(integrand);
    GIAmp1.SetFunction(functor);

    double rlow=1e-3;
    double rhigh=100;
    double result=GIAmp1.Integral(rlow,rhigh); //fm2
    if(std::isnan(result) || std::isinf(result)) return 0.;
    return result;
}
double InclusiveDiffractiveCrossSectionsIntegrals::ComputeAmplitude0Core(
        double r, double b, double beta, double xpom, double z, double Q2, double mf) const {
    if(r>100.) return 0; //Gauss nodes/IntegralUp sometimes probe values that break the dipole model
    double MX2=Q2*(1-beta)/beta;
    double kappa2=z*(1-z)*MX2-mf*mf;
    double kappa=sqrt(kappa2);
    double epsilon2=z*(1-z)*Q2+mf*mf;
    double epsilon=sqrt(epsilon2); //GeV
    double besselK0=TMath::BesselK0(epsilon*r/hbarc);
    double besselJ0=TMath::BesselJ0(kappa*r/hbarc);
    double dsigmadb2=0;
    if(mA==1)
        dsigmadb2=mDipoleModel->dsigmadb2ep(r, b, xpom);
    else{
        dsigmadb2=mDipoleModel->coherentDsigmadb2(r, b, xpom);
    }
    double result = r*besselK0*besselJ0*dsigmadb2; //fm
    if (std::isnan(result) || std::isinf(result)) return 0.;
    return result;
}

double InclusiveDiffractiveCrossSectionsIntegrals::ComputeAmplitude1Core(
        double r, double b, double beta, double xpom, double z, double Q2, double mf) const {
    if(r>100.) return 0; //Gauss nodes/IntegralUp sometimes probe values that break the dipole model
    double MX2=Q2*(1-beta)/beta; //GeV2
    double kappa2=z*(1-z)*MX2-mf*mf; //GeV2
    double kappa=sqrt(kappa2); //GeV
    double epsilon2=z*(1-z)*Q2+mf*mf;
    double epsilon=sqrt(epsilon2); //GeV
    double besselK1=TMath::BesselK1(epsilon*r/hbarc);
    double besselJ1=TMath::BesselJ1(kappa*r/hbarc);
    double dsigmadb2=0; //GeV0
    if(mA==1)
        dsigmadb2=mDipoleModel->dsigmadb2ep(r, b, xpom);
    else
        dsigmadb2=mDipoleModel->coherentDsigmadb2(r, b, xpom);

    double result = r*besselK1*besselJ1*dsigmadb2; //fm
    if (std::isnan(result) || std::isinf(result)) return 0.;
    return result;
}

//===================================================
//           QQG Calculations
//===================================================

double InclusiveDiffractiveCrossSectionsIntegrals::dsigmadbetadz_QQG(double beta, double Q2, double W2, double ztilde) {
    double MX2 = Q2*(1-beta)/beta;
    double xpom = Kinematics::xpomeron(0., Q2, W2, sqrt(MX2));
    
    double Phi=Phi_qqg(xpom, ztilde, Q2);
    
    double alpha_S=0.15;
    double ef=quarkCharge[mQuarkIndex];
    
    double betafactor=(1-beta/ztilde)*(1-beta/ztilde)+beta/ztilde*beta/ztilde;
    double result=alpha_S*alpha_em/(2*M_PI*M_PI*Q2)*betafactor*ef*ef*Phi; //GeV-2
    result*=hbarc2; //fm2
    result *= 1e7; //nb
    // cout<< " beta="<<beta<<", Q2="<<Q2<<", W2="<<W2<<", ztilde="<<ztilde<<", result="<<result<<endl;
    if(std::isnan(result) || std::isinf(result)) return 0.;
    return result; //nb
}

double InclusiveDiffractiveCrossSectionsIntegrals::Phi_qqg(double xpom, double z, double Q2){

    if (gPhiQQGCache.filled &&
        gPhiQQGCache.xpom == xpom &&
        gPhiQQGCache.z    == z &&
        gPhiQQGCache.Q2   == Q2) {
        return gPhiQQGCache.value;
    }

    // Set the state uiPhi_qqg()/ComputeAmplitudeQQGCore() will read
    // (replaces Ampqqg->SetParameter(0, xpom); Ampqqg->SetParameter(1, z);).
    mAmpQQGXpom = xpom;
    mAmpQQGZ    = z;

    if(mA>1)
        dipoleModel()->createSigma_ep_LookupTable(xpom);

    mQ2=Q2;

    int nregions, neval, fail;
    double integral[1], error[1], prob[1];

    // Cuhre works on the unit cube [0,1]^2; uiPhi_qqg maps x[0],x[1] back
    // to physical (b, k2) and applies the Jacobian internally.
    // cubacores(0, 0);
    Cuhre(2, 1, Cuhre_Integrand_QQG, this, 1, 1e-5, 1e-10, 0, 1000, 500000, 9,
          NULL, NULL, &nregions, &neval, &fail, integral, error, prob);

    double result = integral[0]/hbarc; //GeV0
    if(fail != 0 || std::isnan(result) || std::isinf(result)) result = 0.;

    gPhiQQGCache.xpom = xpom;
    gPhiQQGCache.z    = z;
    gPhiQQGCache.Q2   = Q2;
    gPhiQQGCache.value = result;
    gPhiQQGCache.filled = true;

    return result;
}

double InclusiveDiffractiveCrossSectionsIntegrals::uiPhi_qqg(const double* var) {
    // Map Cuba's x[0], x[1] in [0,1] to physical bounds (b, k2)
    double bRange = 3. * dipoleModel()->nucleus()->radius();
    double b  = var[0] * bRange;
    double k2 = 1e-4 + var[1] * (mQ2 - 1e-4);
    double jacobian = bRange * (mQ2 - 1e-4);

    double Q2 = mQ2;

    // Pure-C++ integrand, no TF1/WrappedTF1 in the loop.
    AmpQQG_Integrand integrand{this, mAmpQQGXpom, mAmpQQGZ, sqrt(k2), b};
    ROOT::Math::Functor1D functor(integrand);
    GIAmpqqg.SetFunction(functor);

    double rlow=1e-3;
    double rhigh=100;
    double amp_qqg=GIAmpqqg.Integral(rlow,rhigh); //fm2
    amp_qqg/=hbarc2; //GeV-2

    double result = (2*M_PI)*b/hbarc*k2*k2*log(Q2/k2)*amp_qqg*amp_qqg; //GeV-1
    result *= jacobian; // Jacobian for the [0,1]^2 -> (b,k2) change of variables

    if (std::isnan(result) || std::isinf(result)) return 0.;
    return result; //GeV-1
}

int InclusiveDiffractiveCrossSectionsIntegrals::Cuhre_Integrand_QQG(
        const int *ndim, const double x[], const int *ncomp, double f[], void *userdata) {
    auto* self = static_cast<InclusiveDiffractiveCrossSectionsIntegrals*>(userdata);
    f[0] = self->uiPhi_qqg(x);
    return 0;
}

double InclusiveDiffractiveCrossSectionsIntegrals::ComputeAmplitudeQQGCore(
        double r, double xpom, double z, double k, double b) const {
    if(r>100.) return 0; //Gauss nodes/IntegralUp sometimes probe values that break the dipole model
    double besselK2=ROOT::Math::cyl_bessel_k(2, sqrt(z)*k*r/hbarc);
    double besselJ2=ROOT::Math::cyl_bessel_j(2, sqrt(1-z)*k*r/hbarc);
    double dsigmadb2=0; //GeV0
    if(mA==1)
        dsigmadb2=mDipoleModel->dsigmadb2ep(r, b, xpom);
    else
        dsigmadb2=mDipoleModel->coherentDsigmadb2(r, b, xpom);
    double term=1-0.5*dsigmadb2;
    double dsigmadb2tilde=2*(1-term*term);
    double result=r*besselK2*besselJ2*dsigmadb2tilde; //fm
    if (std::isnan(result) || std::isinf(result)) return 0.;
    return result;
}

// ---------------------------------------------------------------------------
// DEAD CODE: InclusiveDiffractiveCrossSections::uiAmplitude_0/1/qqg below are
// no longer called anywhere in this file now that Amplitude_0/Amplitude_1/
// uiPhi_qqg use the pure-C++ ComputeAmplitude0Core/ComputeAmplitude1Core/
// ComputeAmplitudeQQGCore functions above instead of TF1. Left in place
// because their declarations live in InclusiveDiffractiveCrossSections.h,
// which wasn't available to check for other callers/derived classes. Once
// you confirm nothing else uses them, delete these three definitions and
// their declarations in the base header.
// ---------------------------------------------------------------------------

double InclusiveDiffractiveCrossSections::uiAmplitude_0(double *var, double *par){
    double r=*var;
    if(r>100.) return 0; //IntegralUp sometimes tries too large values that break the dipole model
    double beta=par[0];
    double xpom=par[1];
    double z=par[2];
    double Q2=par[3];
    double b=par[4];
    double mf=par[5];
    double MX2=Q2*(1-beta)/beta;
    double kappa2=z*(1-z)*MX2-mf*mf;
    double kappa=sqrt(kappa2);
    double epsilon2=z*(1-z)*Q2+mf*mf;
    double epsilon=sqrt(epsilon2); //GeV
    double besselK0=TMath::BesselK0(epsilon*r/hbarc);
    double besselJ0=TMath::BesselJ0(kappa*r/hbarc);
    double dsigmadb2=0;
    if(mA==1)
        dsigmadb2=mDipoleModel->dsigmadb2ep(r, b, xpom);
    else{
        dsigmadb2=mDipoleModel->coherentDsigmadb2(r, b, xpom);
    }
    double result = r*besselK0*besselJ0*dsigmadb2; //fm
    if (std::isnan(result) || std::isinf(result)) return 0.;
    return result;
}

double InclusiveDiffractiveCrossSections::uiAmplitude_1(double *var, double *par){
    double r=*var; //fm
    if(r>100.) return 0; //IntegralUp sometimes tries too large values that break the dipole model
    double beta=par[0];
    double xpom=par[1];
    double z=par[2];
    double Q2=par[3]; //GeV2
    double b=par[4];
    double mf=par[5];
    double MX2=Q2*(1-beta)/beta; //GeV2
    double kappa2=z*(1-z)*MX2-mf*mf; //GeV2
    double kappa=sqrt(kappa2); //GeV
    double epsilon2=z*(1-z)*Q2+mf*mf;
    double epsilon=sqrt(epsilon2); //GeV
    double besselK1=TMath::BesselK1(epsilon*r/hbarc);
    double besselJ1=TMath::BesselJ1(kappa*r/hbarc);
    double dsigmadb2=0; //GeV0
    if(mA==1)
        dsigmadb2=mDipoleModel->dsigmadb2ep(r, b, xpom);
    else
        dsigmadb2=mDipoleModel->coherentDsigmadb2(r, b, xpom);

    double result = r*besselK1*besselJ1*dsigmadb2; //fm
    if (std::isnan(result) || std::isinf(result)) return 0.;
    return result;
}

double InclusiveDiffractiveCrossSections::uiAmplitude_qqg(double *var, double *par){
    double r=*var; //fm
    if(r>100.) return 0; //IntegralUp sometimes tries too large values that break the dipole model
    double xpom=par[0];
    double z=par[1];
    double k=par[2]; //GeV
    double b=par[3];
    
    double besselK2=ROOT::Math::cyl_bessel_k(2, sqrt(z)*k*r/hbarc);
    double besselJ2=ROOT::Math::cyl_bessel_j(2, sqrt(1-z)*k*r/hbarc);
    double dsigmadb2=0; //GeV0
    if(mA==1)
        dsigmadb2=mDipoleModel->dsigmadb2ep(r, b, xpom);
    else
        dsigmadb2=mDipoleModel->coherentDsigmadb2(r, b, xpom);
    double term=1-0.5*dsigmadb2;
    double dsigmadb2tilde=2*(1-term*term);
    double result=r*besselK2*besselJ2*dsigmadb2tilde; //fm
    if (std::isnan(result) || std::isinf(result)) {
        return 0.0;
    }   
    return result;
}