//==============================================================================
//  TableGeneratorSettings.cpp
//
//  Copyright (C) 2010-2024 Tobias Toll and Thomas Ullrich
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
//  $Date: 2024-12-14 11:52:39 +0530 (Sat, 14 Dec 2024) $
//  $Author: ttoll $
//==============================================================================
#include "Settings.h"    
#include "TableGeneratorSettings.h"    
#include "Constants.h"    
#include <cmath>    
#include <ctime>
#include <vector>
#include <stdlib.h>

using namespace std;

TableGeneratorSettings* TableGeneratorSettings::mInstance = 0;  // initialize static    
  
TableGeneratorSettings* TableGeneratorSettings::instance()    
{    
    if (mInstance == 0)     
        mInstance = new TableGeneratorSettings;    
    return mInstance;    
}    
  
TableGeneratorSettings::TableGeneratorSettings()    
{    
    //
    // Register all the parameters that can be defined
    // via a runcard.
    // Arguments for registerParameter():
    //     1. pointer to data memeber
    //     2. text string to be used in the runcard
    //     3. default parameter set
    //
    registerParameter(&mBSatLookupPath, "bSatLookupPath", string("./"));
    
    registerParameter(&mTmin, "tmin", -2.);
    registerParameter(&mTmax, "tmax", 0.);
    
    registerParameter(&mXmin, "xmin", 1e-9); //UPC
    registerParameter(&mXmax, "xmax", 3e-2); //UPC

    registerParameter(&mQ2bins, "Q2bins", static_cast<unsigned int>(1));
    registerParameter(&mW2bins, "W2bins", static_cast<unsigned int>(1));
    registerParameter(&mTbins, "tbins",  static_cast<unsigned int>(1));
    registerParameter(&mXbins, "xbins",  static_cast<unsigned int>(1)); //UPC
    registerParameter(&mBetabins, "betabins",  static_cast<unsigned int>(1)); //inc.diff.
    registerParameter(&mZbins, "zbins",  static_cast<unsigned int>(1)); //inc.diff.

    registerParameter(&mNumberOfConfigurations, "numberOfConfigurations", static_cast<unsigned int>(1000));
    vector<double> vec;
    registerParameter(&mDipoleModelCustomParameters, "dipoleModelCustomParameters", vec);
    registerParameter(&mUseBackupFile, "useBackupFile", false);
    registerParameter(&mStartingBinFromBackup, "startingBinFromBackup", 0);
    
    registerParameter(&mStartBin, "startBin", -1);
    registerParameter(&mEndBin, "endBin", -1);
    
    registerParameter(&mModesToCalculate, "modesToCalculate", 0);
    
    registerParameter(&mPriority, "priority", 0);
    
    registerParameter(&mHasSubstructure, "hasSubstructure", false);

    registerParameter(&mFractionOfBinsToFill, "fractionOfBinsToFill", 1.);

}    

void TableGeneratorSettings::consolidateSettings() // called after runcard is read    
{    
    //
    //  Kinematic limits
    //
    if (mQ2min>=mQ2max && !mUPC) {
        cout << "TableGeneratorSettings::consolidateSettings(): Error, Q2min >= Q2max. Stopping" << endl;
        exit(1);
    }
    if (mWmin>=mWmax && !mUPC) {
        cout << "TableGeneratorSettings::consolidateSettings(): Error, Wmin >= Wmax. Stopping" << endl;
        exit(1);
    }
    if (mTmin>=mTmax) {
        cout << "TableGeneratorSettings::consolidateSettings(): Error, tmin >= tmax. Stopping" << endl;
        exit(1);
    }
    if (mTmin>0. || mTmax >0.) {
        cout << "TableGeneratorSettings::consolidateSettings(): Error, t must be negative, please change t-limits. Stopping" << endl;
        exit(1);
    }
    if (mXmin>=mXmax && mUPC) {
        cout << "TableGeneratorSettings::consolidateSettings(): Error, xmin >= xmax. Stopping" << endl;
        exit(1);
    }
    if ((mXmin>=1 || mXmin<=0 || mXmax>=1 || mXmax<=0) && mUPC) {
        cout << "TableGeneratorSettings::consolidateSettings(): Error, xmin or xmax out of range. Stopping" << endl;
        exit(1);
    }
    if (mXmax>0.01 && mUPC){
        cout << "TableGeneratorSettings::consolidateSettings(): Warning, xmax>1e-2, model may be unreliable." << endl;
    }
    if (mA==1 && !mHasSubstructure) mNumberOfConfigurations = 1;
    
    if (!mUseBackupFile) mStartingBinFromBackup = 0;
    
    if (!mUPC)
        mXbins=1;
    else {
        mQ2bins=1;
        mW2bins=1;
    }
    if (mStartBin >= 0 && mEndBin >= 0 && mStartBin>mEndBin) {
        cout << "TableGeneratorSettings::consolidateSettings(): Error, endBin < startBin : " << mEndBin << " <= " << mStartBin << "! Stopping." << endl;
        exit(1);
    }
    if ( mStartBin < 0 ) mStartBin=0;
    if ( mStartBin >= signed(mQ2bins*mW2bins*mTbins*mXbins) ) {
        cout << "TableGeneratorSettings::consolidateSettings(): Error, starting bin >= table! Stopping." << endl;
        exit(1);
    }
    if ( mEndBin > signed(mQ2bins*mW2bins*mTbins*mXbins) || mEndBin < 0) {
        cout << "TableGeneratorSettings::consolidateSettings(): endBin is set to table size=" << mQ2bins*mW2bins*mTbins << endl;
        mEndBin=mQ2bins*mW2bins*mTbins*mXbins*mBetabins*mZbins;
    }
    if ( mModesToCalculate < 0 || mModesToCalculate > 2 ) {
        cout << "TableGeneratorSettings::consolidateSettings(): Error, modesToCalculate can only take values 0, 1, or 2; not "
        << mModesToCalculate << endl;
        exit(1);
    }
    
    //
    //  Make sure the W range is allowed
    //
    double VMMass=lookupPDG(mVectorMesonId)->Mass();
    double W2min=VMMass*VMMass+protonMass2;
    double Wmin=sqrt(W2min);
    if (mWmin<Wmin){
        mWmin=Wmin;
        cout << "TableGeneratorSettings::consolidateSettings(): Warning, Wmin is smaller than allowed value." << endl;
        cout << "                                               It has been changed to Wmin=" << mWmin << endl;
    }
}

//    
//   Access functions     
//    

void TableGeneratorSettings::setTmax(double val){ mTmax=val; }  
double TableGeneratorSettings::tmax() const { return mTmax; }  
  
void TableGeneratorSettings::setTmin(double val){ mTmin=val; }  
double TableGeneratorSettings::tmin() const { return mTmin; }  
  
void TableGeneratorSettings::setXmax(double val){ mXmax=val; }  
double TableGeneratorSettings::xmax() const { return mXmax; }  
  
void TableGeneratorSettings::setXmin(double val){ mXmin=val; }  
double TableGeneratorSettings::xmin() const { return mXmin; }  

void TableGeneratorSettings::setQ2bins(unsigned int val){ mQ2bins=val; }
unsigned int TableGeneratorSettings::Q2bins() const { return mQ2bins; }  
  
void TableGeneratorSettings::setW2bins(unsigned int val){ mW2bins=val; }  
unsigned int TableGeneratorSettings::W2bins() const { return mW2bins; }  
  
void TableGeneratorSettings::setTbins(unsigned int val){ mTbins=val; }  
unsigned int TableGeneratorSettings::tbins() const { return mTbins; }  
    
void TableGeneratorSettings::setXbins(unsigned int val){ mXbins=val; }  
unsigned int TableGeneratorSettings::xbins() const { return mXbins; }  

void TableGeneratorSettings::setBetabins(unsigned int val){ mBetabins=val; }
unsigned int TableGeneratorSettings::betabins() const { return mBetabins; }

void TableGeneratorSettings::setZbins(unsigned int val){ mZbins=val; }
unsigned int TableGeneratorSettings::zbins() const { return mZbins; }

void TableGeneratorSettings::setBSatLookupPath(string val){ mBSatLookupPath = val; }
string TableGeneratorSettings::bSatLookupPath() const { return mBSatLookupPath; }  
  
void TableGeneratorSettings::setNumberOfConfigurations(unsigned int val){ mNumberOfConfigurations=val; }  
unsigned int TableGeneratorSettings::numberOfConfigurations() const { return mNumberOfConfigurations; }  

vector<double> TableGeneratorSettings::dipoleModelCustomParameters() const {return mDipoleModelCustomParameters;}

void TableGeneratorSettings::setUseBackupFile(bool val){ mUseBackupFile = val; }  
bool TableGeneratorSettings::useBackupFile() const { return mUseBackupFile; }  
  
void TableGeneratorSettings::setStartingBinFromBackup(int val){ mStartingBinFromBackup = val; }  
int TableGeneratorSettings::startingBinFromBackup() const { return mStartingBinFromBackup; }  
  
void TableGeneratorSettings::setStartBin(int val){ mStartBin = val; }  
int  TableGeneratorSettings::startBin() const { return mStartBin; }  
  
void TableGeneratorSettings::setEndBin(int val){ mEndBin = val; }  
int TableGeneratorSettings::endBin() const{ return mEndBin; }  
  
void TableGeneratorSettings::setModesToCalculate(int val){ mModesToCalculate = val; }  
int TableGeneratorSettings::modesToCalculate() const { return mModesToCalculate; }  

void TableGeneratorSettings::setPriority(unsigned char val){ mPriority = val; }
unsigned char TableGeneratorSettings::priority() const { return mPriority; }

void TableGeneratorSettings::setHasSubstructure(bool val){ mHasSubstructure = val; }
bool TableGeneratorSettings::hasSubstructure() const { return mHasSubstructure; }

void TableGeneratorSettings::setFractionOfBinsToFill(double val){ mFractionOfBinsToFill = val; }
double TableGeneratorSettings::fractionOfBinsToFill() const { return mFractionOfBinsToFill; }

