//==============================================================================
//  Amplitudes.cpp
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
//  $Date: 2023-06-16 20:27:35 +0530 (Fri, 16 Jun 2023) $
//  $Author: ttoll $
//==============================================================================
//#define SARTRE_IN_MULTITHREADED_MODE 1  
#include <iostream>  
#include <cstdio>  
#include "Amplitudes.h"  
#include "Constants.h"  
#include "TableGeneratorSettings.h"  
#include "DglapEvolution.h"  
#include "Enumerations.h"  
#include "Kinematics.h"  
#include "Integrals.h"  
#include "DipoleModel.h"  
#if defined(SARTRE_IN_MULTITHREADED_MODE)
#include <boost/thread.hpp>  
#endif   
#define PR(x) cout << #x << " = " << (x) << endl;   
  
using namespace std;  
  
Amplitudes::Amplitudes()
{
    mAmplitudeT = 0;
    mAmplitudeL = 0;
    mAmplitudeTnum = 0;
    mAmplitudeLnum = 0;
    mAmplitudeT2 = 0;
    mAmplitudeL2 = 0;
    mNumberOfConfigurations = 0;
    mTheModes = 0;
    mA = 0;
    mErrorT = 0;
    mErrorL = 0;
    mErrorT2 = 0;
    mErrorL2 = 0;

    mAmplitudeTForSkewednessCorrection = 0;
    mAmplitudeLForSkewednessCorrection = 0;
    
    TableGeneratorSettings* settings = TableGeneratorSettings::instance();
    mNumberOfConfigurations = settings->numberOfConfigurations();
    mVerbose = settings->verbose();
    //
    // Create a vector containing instances of the Integrals class
    // and initialize them:
    //
    for (int i=0; i<=mNumberOfConfigurations; i++) {
        mIntegrals.push_back(new IntegralsExclusive);
    }
    
    mA = settings->A();
    mUPC = settings->UPC();

    isBNonSat = false;
    if (settings->dipoleModelType() == bNonSat)
      isBNonSat = true;
    //
    // Get the modes to calculate:
    // 0: <A> analytically <A2> averaged over configurations
    // 1: only <A> analytically
    // 2: Both <A> and <A2> averaged over configurations
    //
    mTheModes = settings->modesToCalculate();
}

Amplitudes& Amplitudes::operator=(const Amplitudes& amp)
{  
    if (this != &amp) {
        for (unsigned int i=0; i<mIntegrals.size(); i++)
            delete mIntegrals[i];
        mIntegrals.clear();
        
        mAmplitudeT = amp.mAmplitudeT;
        mAmplitudeL = amp.mAmplitudeL;
        mAmplitudeTnum = amp.mAmplitudeTnum;
        mAmplitudeLnum = amp.mAmplitudeLnum;
        mAmplitudeT2 = amp.mAmplitudeT2;
        mAmplitudeL2 = amp.mAmplitudeL2;
        mNumberOfConfigurations = amp.mNumberOfConfigurations;
        mTheModes = amp.mTheModes;
        mA = amp.mA;
        mErrorT = amp.mErrorT;
        mErrorL = amp.mErrorL;
        mErrorT2 = amp.mErrorT2;
        mErrorL2 = amp.mErrorL2;

	mAmplitudeTForSkewednessCorrection = amp.mAmplitudeTForSkewednessCorrection;
	mAmplitudeLForSkewednessCorrection = amp.mAmplitudeLForSkewednessCorrection;

        for (unsigned int i=0; i<amp.mIntegrals.size(); i++) {    // deep copy
            mIntegrals.push_back(new IntegralsExclusive(*(amp.mIntegrals[i])));
        }
    }
    return *this;
}  

Amplitudes::Amplitudes(const Amplitudes& amp)   
{   
    mAmplitudeT = amp.mAmplitudeT;
    mAmplitudeL = amp.mAmplitudeL;
    mAmplitudeTnum = amp.mAmplitudeTnum;
    mAmplitudeLnum = amp.mAmplitudeLnum;
    mAmplitudeT2 = amp.mAmplitudeT2;
    mAmplitudeL2 = amp.mAmplitudeL2;
    mErrorT = amp.mErrorT;
    mErrorL = amp.mErrorL;
    mErrorT2 = amp.mErrorT2;
    mErrorL2 = amp.mErrorL2;
    mNumberOfConfigurations = amp.mNumberOfConfigurations;
    mTheModes = amp.mTheModes;
    mA = amp.mA;
    mAmplitudeTForSkewednessCorrection = amp.mAmplitudeTForSkewednessCorrection;
    mAmplitudeLForSkewednessCorrection = amp.mAmplitudeLForSkewednessCorrection;

    for (unsigned int i=0; i<amp.mIntegrals.size(); i++) {    // deep copy
        mIntegrals.push_back(new IntegralsExclusive(*(amp.mIntegrals[i])));
    }
}  

Amplitudes::~Amplitudes()  
{  
    for (unsigned int i=0; i<mIntegrals.size(); i++)
        delete mIntegrals[i];
}  

void Amplitudes::generateConfigurations()  
{  
    for (int i=0; i<mNumberOfConfigurations; i++)
        mIntegrals[i]->dipoleModel()->createConfiguration(i);
}  


//void Amplitudes::calculate(double t, double Q2, double W2)
void Amplitudes::calculate(double* kinematicPoint)
{
    double t=0, Q2=0, W2=0, xpom=0;
    if (!mUPC){
        t  = kinematicPoint[0];
        Q2 = kinematicPoint[1];
        W2 = kinematicPoint[2];
    }
    else{
        t    = kinematicPoint[0];
        xpom = kinematicPoint[1];
    }
#if defined(SARTRE_IN_MULTITHREADED_MODE) // multithreaded version
    if (mA == 1 && mNumberOfConfigurations == 1) {
        cout << "Amplitudes::calculate(): Multithreaded mode (SARTRE_IN_MULTITHREADED_MODE)" << endl;
        cout << "                         is not supported for ep (A=1)"<<endl;
        cout << "                         without nucleonsubstructure(mNumberOfConfigurations=1)."<<endl;
        cout << "                         Stopping." << endl;
        exit(1);
    }
    
    //
    //   Create a vector containing the threads:
    //
    std::vector<boost::thread*> vThreads;
    vThreads.clear();
    
    //
    //   Create the thread group:
    //
    boost::thread_group gThreads;
    if (mTheModes==0 || mTheModes == 2){
        //Start loop over configurations, each calculated on a separate thread:
        for (int i=0; i<mNumberOfConfigurations; i++){
            if (!mUPC)
                vThreads.push_back(new boost::thread(boost::ref(*mIntegrals.at(i)),
                                                     t, Q2, W2));
            else
                vThreads.push_back(new boost::thread(boost::ref(*mIntegrals.at(i)),
                                                     t, xpom));
            gThreads.add_thread(vThreads.at(i));
        }
    }
    
    
    //
    //   Calculate coherent cross-section according to eq.(47) in KT arXiv:hep-ph/0304189v3,
    //   this is done in the main thread in parallel with the other threads
    //   and only in eA:
    //
    if (mA>1 && (mTheModes==1 || mTheModes == 0)) {
        if (!mUPC)
            mIntegrals.at(mNumberOfConfigurations)->coherentIntegrals(t, Q2, W2);
        else
            mIntegrals.at(mNumberOfConfigurations)->coherentIntegrals(t, xpom);
    }
    if (mA == 1 && (mTheModes==1 || mTheModes == 0)) {
        if (!mUPC)
            mIntegrals.at(mNumberOfConfigurations)->coherentIntegralsEp(t, Q2, W2);
        else
            mIntegrals.at(mNumberOfConfigurations)->coherentIntegralsEp(t, xpom);
    }
    if (mTheModes==0 || mTheModes == 2) {
        //   Wait for all threads to finish before continuing main thread:
        gThreads.join_all();
        //   Clean up threads
        vThreads.clear();
    }
#else // unforked version
    if ((mTheModes==0 || mTheModes == 2)) {
        //Start loop over configurations:
        for (int i=0; i<mNumberOfConfigurations; i++) {
            if (!mUPC)
                mIntegrals.at(i)->operator()(t, Q2, W2);
            else
                mIntegrals.at(i)->operator()(t, xpom);
        }
    }
    
    //
    //  Calculate coherent cross-section according to eq.(47) in KT arXiv:hep-ph/0304189v3,
    //  (only in eA)
    //
    if (mA>1 && (mTheModes==1 || mTheModes == 0)){
        if (!mUPC)
            mIntegrals.at(mNumberOfConfigurations)->coherentIntegrals(t, Q2, W2);
        else
            mIntegrals.at(mNumberOfConfigurations)->coherentIntegrals(t, xpom);
    }
    if (mA == 1 && (mTheModes==1 || mTheModes == 0)) {
        if (!mUPC)
            mIntegrals.at(mNumberOfConfigurations)->coherentIntegralsEp(t, Q2, W2);
        else
            mIntegrals.at(mNumberOfConfigurations)->coherentIntegralsEp(t, xpom);
    }
#endif
    
    //
    //   Calculate the resulting <A2>:
    //
    double coherentT = 0, coherentL = 0;
    double errCoherentT = 0, errCoherentL = 0;
    if ((mTheModes==0 || mTheModes == 2) || mA==1) {
        double totalT = 0;
        double totalL = 0;
        double err2TotalT = 0, err2TotalL = 0;
        double probabilityCutOff = 1e-6;
        for (int i=0; i<mNumberOfConfigurations; i++) {
            double valimT = mIntegrals.at(i)->integralImT();
            double valreT = mIntegrals.at(i)->integralReT();
            double valimL = mIntegrals.at(i)->integralImL();
            double valreL = mIntegrals.at(i)->integralReL();
            
            double errimT = mIntegrals.at(i)->errorImT();
            double errimL = mIntegrals.at(i)->errorImL();
            double errreT = mIntegrals.at(i)->errorReT();
            double errreL = mIntegrals.at(i)->errorReL();
            
            double probimT = mIntegrals.at(i)->probImT();
            double probimL = mIntegrals.at(i)->probImL();
            double probreT = mIntegrals.at(i)->probReT();
            double probreL = mIntegrals.at(i)->probReL();
            
            if (probimT > probabilityCutOff || probimL > probabilityCutOff ||
                probreT > probabilityCutOff || probreL > probabilityCutOff){
                if (mVerbose) {
                    cout<< "Amplitudes::calculate(): Warning, Integrals may not have reached desired precision" <<endl;
                    //Print out the largest probability:
                    probimT > probreT && probimT > probimL && probimT > probreL ?
                    cout<< "                         The probability for this is "<<probimT<<endl :
                    probreT > probimT && probreT > probimL && probreT > probreL ?
                    cout<< "                         The probability for this is "<<probreT<<endl :
                    probimL > probimT && probimL > probreT && probimL > probreL ?
                    cout<< "                         The probability for this is "<<probimL<<endl :
                    cout<< "                         The probability for this is "<<probreL<<endl;
                }
            }
            
            //
            //  Calculate the averages
            //
            totalT += (valimT*valimT + valreT*valreT);
            totalL += (valimL*valimL + valreL*valreL);
            coherentT += valimT;
            coherentL += valimL;
            
            //
            //  ...and their errors:
            //
            // err2Total = |dtotal/dval|^2*err^2
            // dtotal/dval = 2*val
            //
            err2TotalT += (4*valimT*valimT*errimT*errimT
                           + 4*valreT*valreT*errreT*errreT);
            err2TotalL += (4*valimL*valimL*errimL*errimL
                           + 4*valreL*valreL*errreL*errreL);
            
            errCoherentT += errimT;
            errCoherentL += errimL;
        }    //for
        
        //
        //   Store the results of the second moment of the amplitudes:
        //
        mAmplitudeT2 = totalT/mNumberOfConfigurations;
        mAmplitudeL2 = totalL/mNumberOfConfigurations;
        //...and it's error:
        mErrorT2 = sqrt(err2TotalT)/mNumberOfConfigurations;
        mErrorL2 = sqrt(err2TotalL)/mNumberOfConfigurations;
    }//if (theModes)
    
    //
    //   Store the results and error of the first moment of the amplitudes:
    //
    if (mA>1 && (mTheModes==1 || mTheModes == 0)) {
        double coherentKTT = mIntegrals.at(mNumberOfConfigurations)->integralImT();
        double coherentKTL = mIntegrals.at(mNumberOfConfigurations)->integralImL();
        double errCoherentKTT = mIntegrals.at(mNumberOfConfigurations)->errorImT();
        double errCoherentKTL = mIntegrals.at(mNumberOfConfigurations)->errorImL();
        mAmplitudeT = coherentKTT;
        mAmplitudeL = coherentKTL;
        mErrorT = errCoherentKTT;
        mErrorL = errCoherentKTL;
    }
    else {
        mAmplitudeT = coherentT/mNumberOfConfigurations;
        mAmplitudeL = coherentL/mNumberOfConfigurations;
        mErrorT = errCoherentT/mNumberOfConfigurations;
        mErrorL = errCoherentL/mNumberOfConfigurations;
    }
    if (mA == 1 && mTheModes != 1 && mNumberOfConfigurations == 1){
        if (isBNonSat){
            mAmplitudeTForSkewednessCorrection = mAmplitudeT;
            mAmplitudeLForSkewednessCorrection = mAmplitudeL;
        }
        else {
            mAmplitudeTForSkewednessCorrection = mIntegrals.at(0)->integralTForSkewedness();
            mAmplitudeLForSkewednessCorrection = mIntegrals.at(0)->integralLForSkewedness();
        }
    }
    if(mTheModes==0){
        mAmplitudeTnum=coherentT/mNumberOfConfigurations;
        mAmplitudeLnum=coherentL/mNumberOfConfigurations;
    }
}
