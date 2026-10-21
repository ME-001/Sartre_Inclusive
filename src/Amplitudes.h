//==============================================================================
//  Amplitudes.h
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
//  $Date: 2023-06-15 21:57:07 +0530 (Thu, 15 Jun 2023) $
//  $Author: ttoll $
//==============================================================================
//  
// The Amplitudes class calculate \gamma-A dsigma/dt cross-sections  
// for a given t, Q2, W2 for T and L photons.   
// It also calculates the respective coherent parts of the cross-sections  
//  
// The results are accessed by the public functions:  
// double amplitudeT()  for <A_T>  
// double amplitudeL()  for <A_L>  
// double amplitudeT2() for <|A_T|^2>  
// double amplitudeL2() for <|A_L|^2>  
//  
// Units are <|A|^2> in nb/GeV^2 and <A> in sqrt(nb/GeV^2)  
//  
//===============================================================================  
#ifndef Amplitudes_h  
#define Amplitudes_h  
#include <vector>  
  
using namespace std;  
  
class IntegralsExclusive;  
  
class Amplitudes {  
public:  
    Amplitudes();  
    Amplitudes(const Amplitudes&);  
    ~Amplitudes();  
  
    Amplitudes& operator=(const Amplitudes&);  
  
    //    void calculate(double, double, double);
    void calculate(double*);  
    void generateConfigurations();  
      
    double amplitudeT() const;
    double amplitudeL() const;
    double amplitudeTnum() const;
    double amplitudeLnum() const;
    double amplitudeT2() const;
    double amplitudeL2() const;  
          
    double errorT() const;  
    double errorL() const;  
    double errorT2() const;  
    double errorL2() const;

    double amplitudeTForSkewednessCorrection() const;
    double amplitudeLForSkewednessCorrection() const;
  
private:  
    // vector of instances of the integrals class:  
    vector<IntegralsExclusive*> mIntegrals;  
      
    // these are the results:  
    double mAmplitudeT;
    double mAmplitudeL;
    double mAmplitudeTnum;
    double mAmplitudeLnum;
    double mAmplitudeT2;
    double mAmplitudeL2;  

    double mAmplitudeTForSkewednessCorrection;
    double mAmplitudeLForSkewednessCorrection;
    
    //...and the (absolute) errors:  
    double mErrorT;  
    double mErrorL;  
    double mErrorT2;  
    double mErrorL2;  
  
    int mNumberOfConfigurations;  
    int mTheModes;  
    unsigned int mA;
    bool mUPC;
    bool mVerbose;
    bool isBNonSat;
};  
  
inline double Amplitudes::amplitudeT() const {return mAmplitudeT;}
inline double Amplitudes::amplitudeL() const {return mAmplitudeL;}
inline double Amplitudes::amplitudeTnum() const {return mAmplitudeTnum;}
inline double Amplitudes::amplitudeLnum() const {return mAmplitudeLnum;}
inline double Amplitudes::amplitudeT2() const {return mAmplitudeT2;}
inline double Amplitudes::amplitudeL2() const {return mAmplitudeL2;}  

inline double Amplitudes::amplitudeTForSkewednessCorrection() const {return mAmplitudeTForSkewednessCorrection;}
inline double Amplitudes::amplitudeLForSkewednessCorrection() const {return mAmplitudeLForSkewednessCorrection;}  

inline double Amplitudes::errorT() const {return mErrorT;}  
inline double Amplitudes::errorL() const {return mErrorL;}  
inline double Amplitudes::errorT2() const {return mErrorT2;}  
inline double Amplitudes::errorL2() const {return mErrorL2;}  
  
#endif  
