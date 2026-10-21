//==============================================================================
//  Integrals.h
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
//  
//  This class calculate the integrals over the   
//  unintegrated amplitude for \gamma-A collisions  
//  It calculates four 4dimensional integrals for a given (t, Q2, W2):  
//  Imaginary part for T photon  
//  Real part for T photon  
//  Imaginary part for L photon  
//  Real part for L photon  
//  
//  The results are accessed via:  
//    double integralImT()  
//    double integralReT()  
//    double integralImL()  
//    double integralReL()  
//  
//==============================================================================  
#ifndef Integrals_h  
#define Integrals_h  
  
class WaveOverlap;  
class DipoleModel;  
class TH1F;  
  
class Integrals {  
public:  
    Integrals();  
    virtual ~Integrals();  
    Integrals(const Integrals&);   
    Integrals& operator=(const Integrals&);   
  
    void operator() (double, double, double);  
    void operator() (double, double); //#TT UPC only takes two arguments  
      
    double integralImT() const;  
    double integralImL() const;  
    double integralReT() const;  
    double integralReL() const;  
      
    double errorImT() const;  
    double errorImL() const;  
    double errorReT() const;  
    double errorReL() const;  
  
    double probImT() const;  
    double probImL() const;  
    double probReT() const;  
    double probReL() const;

    double integralTForSkewedness() const;
    double integralLForSkewedness() const;
    double errorTForSkewedness() const;
    double errorLForSkewedness() const;
    
    DipoleModel* dipoleModel() const;
    DipoleModel* dipoleModelForSkewednessCorrection() const;

protected:  
    virtual void calculate() = 0;  
    virtual bool setKinematicPoint(double, double, double) = 0;  
    virtual bool setKinematicPoint(double, double) = 0;  
    virtual void calculateCoherent() = 0;  
    virtual void calculateSkewedness() = 0;
    virtual void calculateEp() = 0;
      
protected:  
    //the wave-overlap:  
    WaveOverlap* mWaveOverlap;  
    //the dipole model:  
    DipoleModel* mDipoleModel;  
    DipoleModel* mDipoleModelForSkewednessCorrection;  
      
    bool         mIsInitialized;  
    double       mRelativePrecisionOfIntegration;  
    double       mMV;  
      
    //The result from each integral:  
    double mIntegralImT;  
    double mIntegralImL;  
    double mIntegralReT;  
    double mIntegralReL;  
    //The (absolute) error from each integral:  
    double mErrorImT;  
    double mErrorImL;  
    double mErrorReT;  
    double mErrorReL;  
    //The probability for each integral:  
    double mProbImT;  
    double mProbImL;  
    double mProbReT;  
    double mProbReL;  

    bool   mVerbose;
    void fillZeroes();  

    bool   mIsUPC;
    bool   mCalculateSkewedness;
    double mIntegralTForSkewedness;
    double mIntegralLForSkewedness;
    double mErrorTForSkewedness;
    double mErrorLForSkewedness;
    double mProbImTForSkewedness;
    double mProbImLForSkewedness;    
};  
  
  
class IntegralsExclusive : public Integrals {  
public:  
    IntegralsExclusive();  
    IntegralsExclusive(const IntegralsExclusive&);  
    IntegralsExclusive& operator=(const IntegralsExclusive&);  
      
    double uiAmplitudeTIm(double, double, double, double, double, double, double);  
    double uiAmplitudeLIm(double, double, double, double, double, double, double);  
    double uiAmplitudeTRe(double, double, double, double, double, double, double);  
    double uiAmplitudeLRe(double, double, double, double, double, double, double);  
    double uiAmplitudeTep(double, double, double, double, double, double);  
    double uiAmplitudeLep(double, double, double, double, double, double);  
    double uiCoherentAmplitudeT(double, double, double, double, double);  
    double uiCoherentAmplitudeL(double, double, double, double, double);  
    void coherentIntegrals(double, double, double);  
    void coherentIntegrals(double, double);
    void coherentIntegralsEp(double, double, double);
    void coherentIntegralsEp(double, double);


    double uiAmplitudeTForSkewedness(double, double, double, double, double, double);  
    double uiAmplitudeLForSkewedness(double, double, double, double, double, double);  
    
public:      
    double kinematicPoint[4];  
      
private:  
    void   calculate();
    void   calculateSkewedness();
    void   calculateCoherent();
    void   calculateEp();
    bool   setKinematicPoint(double, double, double);  
    bool   setKinematicPoint(double, double);  

};  
  
inline double Integrals::integralImT() const { return mIntegralImT; }  
inline double Integrals::integralImL() const { return mIntegralImL; }  
inline double Integrals::integralReT() const { return mIntegralReT; }  
inline double Integrals::integralReL() const { return mIntegralReL; }  
  
inline double Integrals::errorImT() const { return mErrorImT; }  
inline double Integrals::errorImL() const { return mErrorImL; }  
inline double Integrals::errorReT() const { return mErrorReT; }  
inline double Integrals::errorReL() const { return mErrorReL; }  
  
inline double Integrals::probImT() const {return mProbImT; }  
inline double Integrals::probImL() const {return mProbImL; }  
inline double Integrals::probReT() const {return mProbReT; }  
inline double Integrals::probReL() const {return mProbReL; }  

inline double Integrals::integralTForSkewedness() const {return mIntegralTForSkewedness;}
inline double Integrals::integralLForSkewedness() const {return mIntegralLForSkewedness;}

inline double Integrals::errorTForSkewedness() const {return mErrorTForSkewedness;}
inline double Integrals::errorLForSkewedness() const {return mErrorLForSkewedness;}


inline DipoleModel* Integrals::dipoleModel() const { return mDipoleModel; }  
inline DipoleModel* Integrals::dipoleModelForSkewednessCorrection()
  const { return mDipoleModelForSkewednessCorrection; }
  
#endif  
