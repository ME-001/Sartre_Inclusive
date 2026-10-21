//==============================================================================
//  TableGeneratorSettings.h
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
//  $Date: 2024-12-14 11:52:39 +0530 (Sat, 14 Dec 2024) $
//  $Author: ttoll $
//==============================================================================
//  
//  Singleton class  
//  
//==============================================================================  
#ifndef TableGeneratorSettings_h         
#define TableGeneratorSettings_h         
#include "Settings.h"         
#include "Enumerations.h"  
         
using namespace std;         
         
class TableGeneratorSettings : public Settings {         
public:         
    static TableGeneratorSettings* instance();         
         
    void setTmin(double);
    double tmin() const;  
      
    void setTmax(double);  
    double tmax() const;  

    void setXmin(double);
    double xmin() const;  

    void setXmax(double);  
    double xmax() const;  
      
    void setQ2bins(unsigned int);
    unsigned int Q2bins() const;
      
    void setW2bins(unsigned int);  
    unsigned int W2bins() const;  
      
    void setTbins(unsigned int);  
    unsigned int tbins() const;  

    void setXbins(unsigned int);  
    unsigned int xbins() const;  

    void setBetabins(unsigned int);
    unsigned int betabins() const;

    void setZbins(unsigned int);
    unsigned int zbins() const;

    string bSatLookupPath() const;
    void setBSatLookupPath(string);
      
    unsigned int numberOfConfigurations() const;  
    void setNumberOfConfigurations(unsigned int);
    
    vector<double> dipoleModelCustomParameters() const;
    
    bool useBackupFile() const;  
    void setUseBackupFile(bool);  
      
    int startingBinFromBackup() const;  
    void setStartingBinFromBackup(int);  
      
    int startBin() const;  
    void setStartBin(int);  
      
    int endBin() const;  
    void setEndBin(int);  
      
    int modesToCalculate() const;  
    void setModesToCalculate(int);  

    unsigned char priority() const;
    void setPriority(unsigned char);

    bool hasSubstructure() const;
    void setHasSubstructure(bool);

    double fractionOfBinsToFill() const;
    void setFractionOfBinsToFill(double);

    void consolidateSettings();    
      
private:         
    TableGeneratorSettings();         
             
private:         
    static TableGeneratorSettings* mInstance;         
             
private:         
    unsigned int mNumberOfConfigurations;
    
    vector<double>  mDipoleModelCustomParameters;  // developer
    
    bool mUseBackupFile;  
  
    int  mStartingBinFromBackup;
    int  mStartBin, mEndBin;  
    int  mModesToCalculate;  
                   
    double mTmin;
    double mTmax;         
    double mXmin;
    double mXmax;    
                 
    unsigned int mQ2bins;         
    unsigned int mW2bins;         
    unsigned int mTbins;         
    unsigned int mXbins;
    unsigned int mBetabins;
    unsigned int mZbins;
         
    string mBSatLookupPath;
 
    int mPriority;

    bool mHasSubstructure;
    double mFractionOfBinsToFill;
};
         
#endif         
