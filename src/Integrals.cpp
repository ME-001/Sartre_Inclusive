//==============================================================================
//  Integrals.cpp
//
//  Copyright (C) 2010-2019 Tobias Toll and Thomas Ullrich 
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
//  $Date: 2023-06-15 03:59:56 +0530 (Thu, 15 Jun 2023) $
//  $Author: ttoll $
//==============================================================================
#include <iostream>  
#include <cmath>  
#include <algorithm>  
#include "Integrals.h"  
#include "Constants.h"  
#include "Nucleus.h"  
#include "DipoleModel.h"  
#include "AlphaStrong.h"  
#include "Math/IntegratorMultiDim.h"  
#include "Math/Functor.h"  
#include "TMath.h"  
#include "WaveOverlap.h"  
#include "Kinematics.h"  
#include "TableGeneratorSettings.h"  
#include "Enumerations.h"  
#include "IntegrandWrappers.h"  
#include "TF1.h"  
#include "TH1F.h"  
#include "cuba.h"  

#define PRs(x) cout << #x << " = " << scientific << (x) << endl;
#define PR(x) cout << #x << " = " << (x) << endl;

using namespace std;  


Integrals::Integrals()   
{   
    mIsInitialized = false;
    mRelativePrecisionOfIntegration = 0;
    mWaveOverlap = nullptr;
    mDipoleModel = nullptr;
    mDipoleModelForSkewednessCorrection = nullptr;
    mIntegralImT = 0;
    mIntegralImL = 0;
    mIntegralReT = 0;
    mIntegralReL = 0;
    mErrorImT = 0;
    mErrorImL = 0;
    mErrorReT = 0;
    mErrorReL = 0;
    mProbImT = 0;
    mProbImL = 0;
    mProbReT = 0;
    mProbReL = 0;
    
    mIntegralTForSkewedness = 0;
    mIntegralLForSkewedness = 0;
    mErrorTForSkewedness = 0;
    mErrorLForSkewedness = 0;
    
    TableGeneratorSettings* settings = TableGeneratorSettings::instance();
    
    mVerbose = settings->verbose();
    
    int VMId = settings->vectorMesonId();
    mMV = settings->lookupPDG(VMId)->Mass();
    mIsUPC = settings->UPC();
    
    if (VMId==113 || VMId==333 || VMId == 443 || VMId == 553) {
        mWaveOverlap = new WaveOverlapVM;
        mWaveOverlap->setProcess(VMId);
        mWaveOverlap->setWaveOverlapFunctionParameters(VMId);
        if (mVerbose) mWaveOverlap->testBoostedGaussianParameters(VMId);
    }
    else if (VMId==22) {
        mWaveOverlap = new WaveOverlapDVCS;
    }
    else {
        cout << "Integrals::init(): Error, no exclusive production implemented for: "<< VMId << endl;
        exit(1);
    }
    DipoleModelType model = settings->dipoleModelType();
    if (model == bSat) {
        mDipoleModel = new DipoleModel_bSat;
    }
    else if (model == bNonSat){
        mDipoleModel = new DipoleModel_bNonSat;
    }
    else if (model == bCGC) {
        mDipoleModel = new DipoleModel_bCGC;
    }
    else {
        cout << "Integrals::init(): Error, model not implemented: "<< model << endl;
        exit(1);
    }
    mCalculateSkewedness = false;
    if (settings->A()==1 && settings->modesToCalculate()!=1 && settings->numberOfConfigurations()==1 && model==bSat) {
        mCalculateSkewedness = true;
        mDipoleModelForSkewednessCorrection = new DipoleModel_bNonSat;
    }
    mIsInitialized = true;
}  

Integrals::Integrals(const Integrals& integrals)  
{  
    mIsInitialized = integrals.mIsInitialized;
    mRelativePrecisionOfIntegration = integrals.mRelativePrecisionOfIntegration;
    if (typeid(*mWaveOverlap) == typeid(WaveOverlapDVCS))
        mWaveOverlap  = new WaveOverlapDVCS;
    else
        mWaveOverlap  = new WaveOverlapVM;
    if (typeid(*mDipoleModel) == typeid(DipoleModel_bSat))
        mDipoleModel  = new DipoleModel_bSat;
    else if (typeid(*mDipoleModel) == typeid(DipoleModel_bNonSat))
        mDipoleModel = new DipoleModel_bNonSat;
    else
        mDipoleModel  = new DipoleModel_bCGC;
    if (typeid(*mDipoleModelForSkewednessCorrection) == typeid(DipoleModel_bNonSat))
        mDipoleModel  = new DipoleModel_bNonSat;
    mIntegralImT  = integrals.mIntegralImT;
    mIntegralImL  = integrals.mIntegralImL;
    mIntegralReT  = integrals.mIntegralReT;
    mIntegralReL  = integrals.mIntegralReL;
    mErrorImT = integrals.mErrorImT;
    mErrorImL = integrals.mErrorImL;
    mErrorReT = integrals.mErrorReT;
    mErrorReL = integrals.mErrorReL;
    mProbImT = integrals.mProbImT;
    mProbImL = integrals.mProbImL;
    mProbReT = integrals.mProbReT;
    mProbReL = integrals.mProbReL;
    mMV = integrals.mMV;
}  

Integrals& Integrals::operator=(const Integrals& integrals)  
{  
    if (this != &integrals) {
        delete mWaveOverlap;
        delete mDipoleModel;
        delete mDipoleModelForSkewednessCorrection;
        
        if (typeid(*mWaveOverlap) == typeid(WaveOverlapDVCS))
            mWaveOverlap  = new WaveOverlapDVCS;
        else
            mWaveOverlap  = new WaveOverlapVM;
        if (typeid(*mDipoleModel) == typeid(DipoleModel_bSat))
            mDipoleModel  = new DipoleModel_bSat;
        else if (typeid(*mDipoleModel) == typeid(DipoleModel_bNonSat))
            mDipoleModel  = new DipoleModel_bNonSat;
        else
            mDipoleModel  = new DipoleModel_bCGC;
	if (typeid(*mDipoleModelForSkewednessCorrection) == typeid(DipoleModel_bNonSat))
	    mDipoleModel  = new DipoleModel_bNonSat;
	
        mIntegralImT  = integrals.mIntegralImT;
        mIntegralImL  = integrals.mIntegralImL;
        mIntegralReT  = integrals.mIntegralReT;
        mIntegralReL  = integrals.mIntegralReL;
        mIntegralImT  = integrals.mIntegralImT;
        mIntegralImL  = integrals.mIntegralImL;
        mIntegralReT  = integrals.mIntegralReT;
        mIntegralReL  = integrals.mIntegralReL;
        mErrorImT = integrals.mErrorImT;
        mErrorImL = integrals.mErrorImL;
        mErrorReT = integrals.mErrorReT;
        mErrorReL = integrals.mErrorReL;
        mProbImT = integrals.mProbImT;
        mProbImL = integrals.mProbImL;
        mProbReT = integrals.mProbReT;
        mProbReL = integrals.mProbReL;
        mMV = integrals.mMV;
        mIsInitialized = integrals.mIsInitialized;
        mRelativePrecisionOfIntegration = integrals.mRelativePrecisionOfIntegration;
    }
    return *this;
}  

Integrals::~Integrals()   
{   
    delete mWaveOverlap;
    delete mDipoleModel;
    if (mDipoleModelForSkewednessCorrection)
      delete mDipoleModelForSkewednessCorrection;
}  

void Integrals::operator() (double t, double Q2, double W2)  
{  
    unsigned int A = dipoleModel()->nucleus()->A();
    TableGeneratorSettings* settings = TableGeneratorSettings::instance();
    unsigned int numberOfConfigurations=settings->numberOfConfigurations();
    //make sure the configurations have been generated:
    if (!mDipoleModel->configurationExists() && (A!=1 || (A==1 && numberOfConfigurations>1)) ) {
        // do not use cout
        cout << "Integrals::init(): Error, configuration has not been generated. Stopping." << endl;
        exit(1);
    }
    if (setKinematicPoint(t, Q2, W2)) {
        if (A==1 && numberOfConfigurations == 1){
            calculateEp();
            if (mCalculateSkewedness){
                calculateSkewedness();
            }
        }
        else
            calculate();
    }
    else {
        fillZeroes();
    }
}

void Integrals::operator() (double t, double xpom) //UPC
{
    TableGeneratorSettings* settings = TableGeneratorSettings::instance();
    unsigned int numberOfConfigurations=settings->numberOfConfigurations();
    unsigned int A = dipoleModel()->nucleus()->A();
    //make sure the configurations have been generated:
    if (!mDipoleModel->configurationExists() && (A!=1 || (A==1 && numberOfConfigurations>1)) ) {
        // do not use cout
        cout << "Integrals::init(): Error, configuration has not been generated. Stopping." << endl;
        exit(1);
    }
    if (setKinematicPoint(t, xpom)) {
        if (A==1 && numberOfConfigurations == 1){
            calculateEp();
            if (mCalculateSkewedness){
                calculateSkewedness();
            }
        }
        else
            calculate();
    }
    else {
        fillZeroes();
    }
}

void Integrals::fillZeroes(){  
    //Store the results
    mIntegralImT = 0;
    mIntegralReT = 0;
    mIntegralImL = 0;
    mIntegralReL = 0;
    
    //Store the errors:
    mErrorImT = 0;
    mErrorImL = 0;
    mErrorReT = 0;
    mErrorReL = 0;
    
    //Store the probabilities:
    mProbImT = 0;
    mProbImL = 0;
    mProbReT = 0;
    mProbReL = 0;
    
}  

//*********EXCLUSIVE VECTOR MESONS OR DVCS: ********************************  

void IntegralsExclusive::coherentIntegrals(double t, double Q2, double W2)  
{  
    if (setKinematicPoint(t, Q2, W2)){
        if (typeid(*mDipoleModel) == typeid(DipoleModel_bSat)){
            //store present kinematic point:
            double xprobe = kinematicPoint[3];
            dipoleModel()->createSigma_ep_LookupTable(xprobe);
        }
        calculateCoherent();
    }
    else
        fillZeroes();
}  

void IntegralsExclusive::coherentIntegrals(double t, double xpom)  
{  
    if (setKinematicPoint(t, xpom)){
        if (typeid(*mDipoleModel) == typeid(DipoleModel_bSat)){
            dipoleModel()->createSigma_ep_LookupTable(xpom);
        }
        calculateCoherent();
    }
    else
        fillZeroes();
}  

void IntegralsExclusive::coherentIntegralsEp(double t, double Q2, double W2)
{
    if (setKinematicPoint(t, Q2, W2))
        calculateEp();
    else
        fillZeroes();
}

void IntegralsExclusive::coherentIntegralsEp(double t, double xpom)
{
    if (setKinematicPoint(t, xpom))
        calculateEp();
    else
        fillZeroes();
}

IntegralsExclusive::IntegralsExclusive()   
{  
}  

IntegralsExclusive& IntegralsExclusive::operator=(const IntegralsExclusive& cobj)  
{    
    if (this != &cobj) {
        Integrals::operator=(cobj);
        copy(cobj.kinematicPoint, cobj.kinematicPoint+4, kinematicPoint);
    }
    return *this;
}  

IntegralsExclusive::IntegralsExclusive(const IntegralsExclusive& cobj) : Integrals(cobj)  
{  
    copy(cobj.kinematicPoint, cobj.kinematicPoint+4, kinematicPoint);
}  


bool IntegralsExclusive::setKinematicPoint(double t, double xpom) //UPC
{
    bool result = true;
    kinematicPoint[0] = t;
    kinematicPoint[1] = 0; //Q2
    kinematicPoint[2] = 0; //W2 is not used
    kinematicPoint[3] = xpom;
    
    return result;
}  

bool IntegralsExclusive::setKinematicPoint(double t, double Q2, double W2)
{
    bool result = true;
    kinematicPoint[0] = t;
    kinematicPoint[1] = Q2;
    kinematicPoint[2] = W2;
    double xprobe = Kinematics::xpomeron(t, Q2, W2, mMV);
    
    if (xprobe<0 || xprobe>1)
        result = false;
    kinematicPoint[3] = xprobe;
    return result;
}  


void IntegralsExclusive::calculate()  
{  
    //
    // This function calls a wrapper from where the
    // integral is calculated with the Cuhre method.
    // Pass this Integrals object as the fourth (void*) argument of the Cuhre function.
    //
    const double epsrel = 1.e-2, epsabs = 1e-12;
    const int flags=0, mineval=3e6, maxeval=1e9, key=0;
    int nregionsTIm, nevalTIm, failTIm;
    int nregionsTRe, nevalTRe, failTRe;
    int nregionsLIm, nevalLIm, failLIm;
    int nregionsLRe, nevalLRe, failLRe;
    double valTIm=0, errTIm=0, probTIm=0;
    double valLIm=0, errLIm=0, probLIm=0;
    double valTRe=0, errTRe=0, probTRe=0;
    double valLRe=0, errLRe=0, probLRe=0;
    
    const char* statefile=0;
    
    const int nvec=1;

    //    double probabilityCutOff=1e-6;
    
    //
    //   Do the integrations
    //
    Cuhre(4, 1, integrandWrapperTIm, this, nvec,
          epsrel, epsabs, flags,
          mineval, maxeval, key, statefile, 0, &nregionsTIm, &nevalTIm, &failTIm, &valTIm, &errTIm, &probTIm);
    if (failTIm != 0 && mVerbose) {
        cout << "IntegralsExclusive::calculate(): Warning: Integration TIm did not reach desired precision! Error code=" << failTIm << endl;
    }

    //
    // For UPC, calculate only transverse polarisation case
    //
    if (!mIsUPC) {
        Cuhre(4, 1, integrandWrapperLIm, this, nvec,
              epsrel, epsabs, flags,
              mineval, maxeval, key, statefile, 0, &nregionsLIm, &nevalLIm, &failLIm, &valLIm, &errLIm, &probLIm);
        if (failLIm!=0 && mVerbose) {
            cout << "IntegralsExclusive::calculate(): Warning: Integration LIm did not reach desired precision! Error code=" << failLIm << endl;
        }
    }
    Cuhre(4, 1, integrandWrapperTRe, this, nvec,
          epsrel, epsabs, flags,
          mineval, maxeval, key, statefile, 0, &nregionsTRe, &nevalTRe, &failTRe, &valTRe, &errTRe, &probTRe);
    if (failTRe!=0 && mVerbose) {
        cout << "IntegralsExclusive::calculate(): Warning: Integration TRe did not reach desired precision! Error code=" << failTRe << endl;
    }
    
    //
    // For UPC, calculate only transverse polarisation case
    //
    if (!mIsUPC) {
        Cuhre(4, 1, integrandWrapperLRe, this, nvec,
              epsrel, epsabs, flags,
              mineval, maxeval, key, statefile, 0, &nregionsLRe, &nevalLRe, &failLRe, &valLRe, &errLRe, &probLRe);
        if (failLRe!=0 && mVerbose) {
            cout << "IntegralsExclusive::calculate(): Warning: Integration LRe did not reach desired precision! Error code=" << failLRe << endl;
        }
    }
    
    //
    //   Store the results:
    //
    mIntegralImT = valTIm;
    mIntegralReT = valTRe;
    mIntegralImL = valLIm;
    mIntegralReL = valLRe;
    
    //
    //   Store the errors:
    //
    mErrorImT = errTIm;
    mErrorImL = errLIm;
    mErrorReT = errTRe;
    mErrorReL = errLRe;
    
    //
    //   Store the probabilities:
    //
    mProbImT = probTIm;
    mProbImL = probLIm;
    mProbReT = probTRe;
    mProbReL = probLRe;
}

void IntegralsExclusive::calculateEp()
{
    //
    // This function calls a wrapper from where the
    // integral is calculated with the Cuhre method.
    // Pass this Integrals object as the fourth (void*) argument of the Cuhre function.
    //
    const double epsrel = 1.e-4, epsabs = 1e-12;
    const int flags=0, maxeval=1e9, key=0;
    const int mineval=3e6;
    int nregionsT, nevalT, failT;
    int nregionsL, nevalL, failL;
    double valT=0, errT=0, probT=0;
    double valL=0, errL=0, probL=0;
    
    const char* statefile=0;
    
    const int nvec=1;
    //
    //   Do the integrations
    //
    Cuhre(4, 1, integrandWrapperTep, this, nvec,
          epsrel, epsabs, flags,
          mineval, maxeval, key, statefile, 0, &nregionsT, &nevalT, &failT, &valT, &errT, &probT);
    if (failT!=0 && mVerbose) {
        cout << "IntegralsExclusive::calculateEp(): Warning: Integration T did not reach desired precision! Error code=" << failT << endl;
    }
    //
    // For UPC, calculate only transverse polarisation case
    //
    if (!mIsUPC){
        Cuhre(4, 1, integrandWrapperLep, this, nvec,
	      epsrel, epsabs, flags,
	      mineval, maxeval, key, statefile, 0, &nregionsL, &nevalL, &failL, &valL, &errL, &probL);
        if (failL!=0 && mVerbose) {
            cout << "IntegralsExclusive::calculateEp(): Warning: Integration L did not reach desired precision! Error code=" << failL << endl;
        }
    }
    //
    //   Store the results:
    //
    mIntegralImT=valT;
    mIntegralImL=valL;
    
    //
    //   Store the errors:
    //
    mErrorImT=errT;
    mErrorImL=errL;
    
    //
    //   Store the probabilities:
    //
    mProbImT=probT;
    mProbImL=probL;
}

void IntegralsExclusive::calculateSkewedness()
{
    //
    // This function calls a wrapper from where the
    // integral is calculated with the Cuhre method.
    // Pass this Integrals object as the fourth (void*) argument of the Cuhre function.
    //
    const double epsrel=1.e-4, epsabs=1e-12;
    const int flags=0, maxeval=1e9, key=0;
    const int  mineval=3e6;
    int nregionsT, nevalT, failT;
    int nregionsL, nevalL, failL;
    double valT=0, errT=0, probT=0;
    double valL=0, errL=0, probL=0;

    const char* statefile=0;
    
    const int nvec=1;

    //
    //   Do the integrations
    //
    /*
    bool bContinue=true;
    bool isFirst=true;
    while(bContinue){
      double valTOld=valT;
      Cuhre(4, 1, integrandWrapperTForSkewedness, this, nvec,
	    epsrel, epsabs, flags,
	    mineval, maxeval, key, statefile, 0, &nregionsT, &nevalT, &failT, &valT, &errT, &probT);
      if (abs(valT-valTOld)/valT > epsrel){
	mineval*=3;
	if (isFirst)
	  mineval*=30;
	isFirst=false;
	if (mineval>1e4)
	  PR(mineval);
      }
      else
	bContinue=false;
    }
    */
    Cuhre(4, 1, integrandWrapperTForSkewedness, this, nvec,
	  epsrel, epsabs, flags,
	  mineval, maxeval, key, statefile, 0, &nregionsT, &nevalT, &failT, &valT, &errT, &probT);
    
    if (failT!=0 && mVerbose) {
        cout << "IntegralsExclusive::calculateSkweedness(): Warning: Integration T did not reach desired precision! Error code" << failT << endl;
    }
    //
    // For UPC, calculate only transverse polarisation case
    //
    if (!mIsUPC) {
        Cuhre(4, 1, integrandWrapperLForSkewedness, this, nvec,
              epsrel, epsabs, flags,
              mineval, maxeval, key, statefile, 0, &nregionsL, &nevalL, &failL, &valL, &errL, &probL);
        if (failL!=0 && mVerbose) {
            cout << "IntegralsExclusive::calculateSkweedness(): Warning: Integration L did not reach desired precision! Error code" << failL << endl;
        }
    }
    //
    //   Store the results:
    //
    mIntegralTForSkewedness=valT;
    mIntegralLForSkewedness=valL;
    //
    //   Store the errors:
    //
    mErrorTForSkewedness=errT;
    mErrorLForSkewedness=errL;
    //
    //   Store the probabilities:
    //
    mProbImTForSkewedness=probT;
    mProbImLForSkewedness=probL;
}

void IntegralsExclusive::calculateCoherent()  
{
    //
    //  As calculate() but for the coherent case
    //
    const double epsrel=1e-5, epsabs=1e-12;
    const int flags=0, mineval=1e1, maxeval=1e9, key=0;
    int nregionsT, nevalT, failT;
    int nregionsL, nevalL, failL;
    double valT, errT, probT;
    double valL=0, errL=0, probL=0;
    const int nvec=1;
    const char* statefile=0;

    //
    //   Do the integrations
    //
    Cuhre(3, 1, integrandWrapperCoherentAmplitudeT, this, nvec,
          epsrel, epsabs, flags,
          mineval, maxeval, key, statefile, 0, &nregionsT, &nevalT, &failT, &valT, &errT, &probT);
    if (failT!=0 && mVerbose) {
        cout << "IntegralsExclusive::calculateCoherent(): Warning: Integration T did not reach desired precision! Error code=" <<  failT << endl;
    }
    //
    // For UPC, calculate only transverse polarisation case
    //
    if (!mIsUPC){
        Cuhre(3, 1, integrandWrapperCoherentAmplitudeL, this, nvec,
              epsrel, epsabs, flags,
              mineval, maxeval, key, statefile, 0, &nregionsL, &nevalL, &failL, &valL, &errL, &probL);
        if (failL!=0 && mVerbose) {
            cout << "IntegralsExclusive::calculateCoherent(): Warning: Integration L did not reach desired precision! Error=" << failL << endl;
        }
    }
    //
    //   Store the results
    //
    mIntegralImT=valT;
    mIntegralImL=valL;
    
    //
    //   Store the errors
    //
    mErrorImT=errT;
    mErrorImL=errL;
}

//  
//   The following functions are the Integrands in the Amplitudes:
//  
double IntegralsExclusive::uiAmplitudeTIm(double b, double z, double r, double phi, double Q2, double xprobe, double Delta)  
{  
    double cosArg = (b/hbarc)*Delta*cos(phi);
    double waveOverlap = mWaveOverlap->T(z, Q2, r);
    double dsigdb2 = dipoleModel()->dsigmadb2(r  , b  , phi, xprobe);
    double BesselJ0 = TMath::BesselJ0((0.5-z)*r*Delta/hbarc);
    double result=0.5*r/hbarc2*waveOverlap*
    BesselJ0*b*cos(cosArg)*dsigdb2;
    return result;
}  

double IntegralsExclusive::uiAmplitudeTRe(double b, double z, double r, double phi, double Q2, double xprobe, double Delta)  
{  
    double sinArg = b*Delta*cos(phi)/hbarc;
    double waveOverlap = mWaveOverlap->T(z, Q2, r);
    double dsigdb2 = dipoleModel()->dsigmadb2(r, b, phi, xprobe);
    double BesselJ0 = TMath::BesselJ0((0.5-z)*r*Delta/hbarc);
    double result=0.5*r/hbarc2*waveOverlap*
    BesselJ0*b*sin(sinArg)*dsigdb2;
    return result;
}  
double IntegralsExclusive::uiAmplitudeLIm(double b, double z, double r, double phi, double Q2, double xprobe, double Delta)  
{  
    double waveOverlap = mWaveOverlap->L(z, Q2, r);
    double cosArg = b*Delta*cos(phi)/hbarc;
    double dsigdb2 = dipoleModel()->dsigmadb2(r, b, phi, xprobe);
    double BesselJ0 = TMath::BesselJ0((0.5-z)*r*Delta/hbarc);
    double result=0.5*r/hbarc2*waveOverlap*
    BesselJ0*b*cos(cosArg)*dsigdb2;
    return result;
}  

double IntegralsExclusive::uiAmplitudeLRe(double b, double z, double r, double phi, double Q2, double xprobe, double Delta)  
{  
    double waveOverlap = mWaveOverlap->L(z, Q2, r);
    double sinArg = b*Delta*cos(phi)/hbarc;
    double dsigdb2 = dipoleModel()->dsigmadb2(r, b, phi, xprobe);
    double BesselJ0 = TMath::BesselJ0((0.5-z)*r*Delta/hbarc);
    double result=0.5*r/hbarc2*waveOverlap*
    BesselJ0*b*sin(sinArg)*dsigdb2;
    return result;
}  

double IntegralsExclusive::uiCoherentAmplitudeT(double b, double z, double r, double Q2, double Delta)  
{  
    double waveOverlap = mWaveOverlap->T(z, Q2, r);
    double BesselJ0r = TMath::BesselJ0((0.5-z)*r*Delta/hbarc);
    double BesselJ0b = TMath::BesselJ0(b*Delta/hbarc);
    double xprobe=kinematicPoint[3];
    double dsigmadb2Mean=dipoleModel()->coherentDsigmadb2(r, b, xprobe);
    double result = M_PI*r*b/hbarc2*waveOverlap*BesselJ0r*BesselJ0b*dsigmadb2Mean;
    return result;
}  

double IntegralsExclusive::uiCoherentAmplitudeL(double b, double z, double r, double Q2, double Delta)  
{  
    double waveOverlap = mWaveOverlap->L(z, Q2, r);
    double BesselJ0r = TMath::BesselJ0((0.5-z)*r*Delta/hbarc);
    double BesselJ0b = TMath::BesselJ0(b*Delta/hbarc);
    double xprobe=kinematicPoint[3];
    double dsigmadb2Mean=dipoleModel()->coherentDsigmadb2(r, b, xprobe);
    double result = M_PI*r*b/hbarc2*waveOverlap*BesselJ0r*BesselJ0b*dsigmadb2Mean;
    return result;
}  

double IntegralsExclusive::uiAmplitudeTep(double b, double z, double r, double Q2, double xprobe, double Delta)  
{  
    double waveOverlap = mWaveOverlap->T(z, Q2, r);
    double dsigdb2 = dipoleModel()->dsigmadb2ep(r , b, xprobe);
    double BesselJ0r = TMath::BesselJ0((0.5-z)*r*Delta/hbarc);
    double BesselJ0b = TMath::BesselJ0(b*Delta/hbarc);
    double result=0.5*r/hbarc2*waveOverlap*BesselJ0r*b*BesselJ0b*dsigdb2;
    result*=2*M_PI; 
    return result;
}
double IntegralsExclusive::uiAmplitudeLep(double b, double z, double r, double Q2, double xprobe, double Delta)  
{  
    double waveOverlap = mWaveOverlap->L(z, Q2, r);
    double dsigdb2 = dipoleModel()->dsigmadb2ep(r , b, xprobe);
    double BesselJ0r = TMath::BesselJ0((0.5-z)*r*Delta/hbarc);
    double BesselJ0b = TMath::BesselJ0(b*Delta/hbarc);
    double result=0.5*r/hbarc2*waveOverlap*BesselJ0r*b*BesselJ0b*dsigdb2;
    result*=2*M_PI; 
    return result;
}

//
// Only for calculating the lamdba for Skewedness Corrections:
//
double IntegralsExclusive::uiAmplitudeTForSkewedness(double b, double z, double r, double Q2, double xprobe, double Delta)  
{  
    double waveOverlap = mWaveOverlap->T(z, Q2, r);
    double dsigdb2 = dipoleModelForSkewednessCorrection()->dsigmadb2ep(r , b, xprobe);
    double BesselJ0r = TMath::BesselJ0((0.5-z)*r*Delta/hbarc);
    double BesselJ0b = TMath::BesselJ0(b*Delta/hbarc);
    double result=0.5*r/hbarc2*waveOverlap*BesselJ0r*b*BesselJ0b*dsigdb2;
    result*=2*M_PI; 
    return result;
}

double IntegralsExclusive::uiAmplitudeLForSkewedness(double b, double z, double r, double Q2, double xprobe, double Delta)  
{  
    double waveOverlap = mWaveOverlap->L(z, Q2, r);
    double dsigdb2 = dipoleModelForSkewednessCorrection()->dsigmadb2ep(r, b, xprobe);
    double BesselJ0r = TMath::BesselJ0((0.5-z)*r*Delta/hbarc);
    double BesselJ0b = TMath::BesselJ0(b*Delta/hbarc);
    double result=0.5*r/hbarc2*waveOverlap*BesselJ0r*b*BesselJ0b*dsigdb2;
    result*=2*M_PI; 
    return result;
}  
