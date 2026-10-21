// //==============================================================================
// //  DipoleModel.h
// //
// //  Copyright (C) 2010-2024 Tobias Toll and Thomas Ullrich 
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
// //  $Date: 2024-06-01 05:06:14 +0530 (Sat, 01 Jun 2024) $
// //  $Author: ullrich $
// //==============================================================================
// #ifndef DipoleModel_h  
// #define DipoleModel_h  

// #include "AlphaStrong.h"  
// #include "TableGeneratorNucleus.h"  
// #include "DipoleModelParameters.h"

// // ROOT Math inclusions for pure C++ integration without TF1
// #include "Math/GaussIntegrator.h"
// #include "Math/Functor.h"

// class TH2F;  
// class TH1F;  
// class DipoleModel_bSat; // 1. Forward declaration

// class DipoleModel {  
// public:  
//     DipoleModel();  
//     virtual ~DipoleModel();  
      
//     const TableGeneratorNucleus* nucleus() const;  
//     bool  configurationExists() const;  
      
//     virtual void   createConfiguration(int) = 0;  
//     virtual double dsigmadb2(double, double, double, double) = 0;  
//     virtual double bDependence(double);
//     virtual double bDependence(double, double);
//     virtual double dsigmadb2ep(double, double, double);  // ep
//     virtual double coherentDsigmadb2(double, double, double) { return 0; };  // eA
//     virtual void   createSigma_ep_LookupTable(double) {/* nothing */};
//     DipoleModelParameters* getParameters() { return mParameters; }
    
// protected:  
//     TableGeneratorNucleus mNucleus;  
//     DipoleModelParameters *mParameters;

//     AlphaStrong mAs;
//     bool        mConfigurationExists;  
//     bool        mIsInitialized;
// };  

// // 2. Struct declaration
// struct DSigmaIntegrationFunctor {
//     DipoleModel_bSat* model;
//     double r;
//     double xprobe;
//     double factor;

//     double operator()(double b) const; // Declaration only
// };

// class DipoleModel_bSat : public DipoleModel {  
// public:  
//     DipoleModel_bSat();
//     DipoleModel_bSat(Settings*);
//     DipoleModel_bSat(const DipoleModel_bSat&);
//     DipoleModel_bSat& operator=(const DipoleModel_bSat&);  
//     ~DipoleModel_bSat();

//     void   createSigma_ep_LookupTable(double);
//     void   createConfiguration(int);
//     double dsigmadb2(double, double, double, double);  
//     double bDependence(double, double);  
//     double dsigmadb2ep(double, double, double);  
//     double coherentDsigmadb2(double, double, double);

// protected:  
//     TH2F*  mBDependence;  
  
// private:  
//     TH1F*                  mSigma_ep_LookupTable;
//     ROOT::Math::GaussIntegrator mGIlookup; // Reusable member integrator
// };

// // 3. Definition placed AFTER DipoleModel_bSat is fully defined
// inline double DSigmaIntegrationFunctor::operator()(double b) const {
//     return factor * b * model->dsigmadb2ep(r, b, xprobe);
// }
  
// class DipoleModel_bNonSat : public DipoleModel_bSat {  
// public:  
//     DipoleModel_bNonSat();
//     DipoleModel_bNonSat(Settings*);
//     ~DipoleModel_bNonSat();
    
//     double dsigmadb2(double, double, double, double);
//     double dsigmadb2ep(double, double, double);  
//     double coherentDsigmadb2(double, double, double);  
// };  

// class DipoleModel_bCGC : public DipoleModel {  
// public:
//     DipoleModel_bCGC();
    
//     void   createConfiguration(int);
//     double dsigmadb2(double, double, double, double);  
//     double dsigmadb2ep(double, double, double);  
//     double bDependence(double);  
// };  

// #endif  

//==============================================================================
//  DipoleModel.h
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
//  $Date: 2024-06-01 05:06:14 +0530 (Sat, 01 Jun 2024) $
//  $Author: ullrich $
//==============================================================================
#ifndef DipoleModel_h  
#define DipoleModel_h  

#include "AlphaStrong.h"  
#include "TableGeneratorNucleus.h"  
#include "DipoleModelParameters.h"

#include "Math/GaussIntegrator.h"
#include "Math/Functor.h"

class TH2F;  
class TH1F;  
class DipoleModel_bSat;

class DipoleModel {  
public:  
    DipoleModel();  
    virtual ~DipoleModel();  
      
    const TableGeneratorNucleus* nucleus() const;  
    bool  configurationExists() const;  
      
    virtual void   createConfiguration(int) = 0;  
    virtual double dsigmadb2(double, double, double, double) = 0;  
    virtual double bDependence(double);
    virtual double bDependence(double, double);
    virtual double dsigmadb2ep(double, double, double);  // ep
    virtual double coherentDsigmadb2(double, double, double) { return 0; };  // eA
    virtual void   createSigma_ep_LookupTable(double) {/* nothing */};
    DipoleModelParameters* getParameters() { return mParameters; }
    
protected:  
    TableGeneratorNucleus mNucleus;  
    DipoleModelParameters *mParameters;

    AlphaStrong mAs;
    bool        mConfigurationExists;  
    bool        mIsInitialized;
};  

struct DSigmaIntegrationFunctor {
    DipoleModel_bSat* model;
    double r;
    double xprobe;
    double factor;

    double operator()(double b) const;
};

class DipoleModel_bSat : public DipoleModel {  
public:  
    DipoleModel_bSat();
    DipoleModel_bSat(Settings*);
    DipoleModel_bSat(const DipoleModel_bSat&);
    DipoleModel_bSat& operator=(const DipoleModel_bSat&);  
    ~DipoleModel_bSat();

    void   createSigma_ep_LookupTable(double);
    void   createConfiguration(int);
    double dsigmadb2(double, double, double, double);  
    double bDependence(double, double);  
    double dsigmadb2ep(double, double, double);  
    double coherentDsigmadb2(double, double, double);

protected:  
    TH2F*  mBDependence;  
  
private:  
    TH1F*                       mSigma_ep_LookupTable;
    ROOT::Math::GaussIntegrator mGIlookup;
    double                      mLastXpom; // Class-level cache key
};

inline double DSigmaIntegrationFunctor::operator()(double b) const {
    return factor * b * model->dsigmadb2ep(r, b, xprobe);
}

class DipoleModel_bNonSat : public DipoleModel_bSat {  
public:  
    DipoleModel_bNonSat();
    DipoleModel_bNonSat(Settings*);
    ~DipoleModel_bNonSat();
    
    double dsigmadb2(double, double, double, double);
    double dsigmadb2ep(double, double, double);  
    double coherentDsigmadb2(double, double, double);  
};  

class DipoleModel_bCGC : public DipoleModel {  
public:
    DipoleModel_bCGC();
    
    void   createConfiguration(int);
    double dsigmadb2(double, double, double, double);  
    double dsigmadb2ep(double, double, double);  
    double bDependence(double);  
};  

#endif  

