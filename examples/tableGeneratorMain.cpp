//==============================================================================
//  tableGeneratorMain.cpp
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
//  Author: Thomas Ullrich
//  Last update: 
//  $Date: 2023-07-26 11:48:08 +0530 (Wed, 26 Jul 2023) $
//  $Author: ttoll $
//==============================================================================
//   
//  Main program to create amplitude lookup tables.   
//  [Developer only]
//
//  Usage:
//     tableGeneratorMain runcard startBin endBin
//
//  Bins run from 0 to nbin-1.
//  Loop fill all bins from startBIn to endBin (including endBin).
//  If endBin exceeds the size of the table it set to the nbin-1.
//
//==============================================================================   
#include <sstream>  
#include <cstdlib>  
#include <iostream>
#include <limits>
#include <cmath>
#include <iomanip>
#include "Amplitudes.h"  
#include "TROOT.h"
#include "TH1D.h"  
#include "TFile.h"  
#include "Nucleus.h"
#include "Constants.h"
#include "Table.h"  
#include "TableGeneratorSettings.h"  
#include "Enumerations.h"  
#include "DglapEvolution.h"
#include "Version.h"
#include "TRandom3.h"


#define PR(x) cout << #x << " = " << (x) << endl;  

using namespace std;  

int main(int argc, char *argv[]) {
    
    //
    //  Print header
    //
    time_t theStartTime = time(0);
    string ctstr(ctime(&theStartTime));
    ctstr.erase(ctstr.size()-1, 1);
    cout << "/========================================================================\\" << endl;
    cout << "|                                                                        |" << endl;
    cout << "|  Sartre, Version " << setw(54) << left << VERSION << right << '|' << endl;
    cout << "|                                                                        |" << endl;
    cout << "|  An event generator for exclusive diffractive vector meson production  |" << endl;
    cout << "|  in ep and eA collisions based on the dipole model.                    |" << endl;
    cout << "|                                                                        |" << endl;
    cout << "|  Copyright (C) 2010-2018 Tobias Toll and Thomas Ullrich                |" << endl;
    cout << "|                                                                        |" << endl;
    cout << "|  This program is free software: you can redistribute it and/or modify  |" << endl;
    cout << "|  it under the terms of the GNU General Public License as published by  |" << endl;
    cout << "|  the Free Software Foundation, either version 3 of the License, or     |" << endl;
    cout << "|  any later version.                                                    |" << endl;
    cout << "|                                                                        |" << endl;
    cout << "|  Code compiled on " << setw(12) << left << __DATE__;
    cout << setw(41) << left << __TIME__ << right << '|' << endl;
    cout << "|  Run started at " << setw(55) << left << ctstr.c_str() << right << '|' << endl;
    cout << "|                                                                        |" << endl;
    cout << "|  --------------------------------------------------------------------  |" << endl;
    cout << "|                                                                        |" << endl;
    cout << "|  Sartre Table Generator (Experts only)                                 |" << endl;
    cout << "|                                                                        |" << endl;
    cout << "\\========================================================================/" << endl;
    
    TH1::AddDirectory(false);  // to explicitly delete all new histograms by hand
    TableGeneratorSettings* settings = TableGeneratorSettings::instance();
    
    //
    //  Check arguments
    //
    char* runcard;
    if (argc != 4) {
        cout << "Usage: tableGeneratorMain runcard startBin endBin" << endl;
        return 2;
    }
    else {
        runcard = argv[1];
        settings->setStartBin(atoi(argv[2]));
        settings->setEndBin(atoi(argv[3]));
    }
    
    cout << "Reading settings from runcard." << endl;
    settings->readSettingsFromFile(runcard);
    settings->consolidateSettings();
    
    int nBinQ2 = settings->Q2bins();
    int nBinW2 = settings->W2bins();
    int nBinT  = settings->tbins();
    double Q2min =  settings->Q2min();
    double Q2max =  settings->Q2max();
    double Wmin =   settings->Wmin();
    double Wmax =   settings->Wmax();
    double W2min =  Wmin*Wmin;
    double W2max =  Wmax*Wmax;
    double tmin =   settings->tmin();
    double tmax =   settings->tmax();
    unsigned int massA = settings->A();
    int vmPDG = settings->vectorMesonId();
    DipoleModelType model = settings->dipoleModelType();
    DipoleModelParameterSet pset = settings->dipoleModelParameterSet();
    int startingBin = settings->startBin();
    int endingBin = settings->endBin();
    int modes = settings->modesToCalculate();
    unsigned char priority = settings->priority();
    bool hotspots = settings->hasSubstructure();
    double fractionOfBinsToFill=settings->fractionOfBinsToFill();
    settings->list();
    
    //
    //   Check if lambda tables can be calculated
    //
    bool createLambdaTables = true;
    if (massA == 1 && modes == 1 && settings->numberOfConfigurations() == 1) {
        cout << "\nLambda tables will be generated." << endl;
    }
    else {
        cout << "\nLambda tables will not be generated. Requires A = 1, mode = 1, and 1 configuration only." << endl;
        createLambdaTables = false;
    }
    
    
    //
    //  Check bins
    //
    //  Table's bin indices run from 0 ... nbins-1
    //
    int maxbins = nBinQ2*nBinW2*nBinT;
    if (endingBin >= maxbins) {
        cout << "Warning, given end bin (" << endingBin << ") exceeds the size of the table." << endl;
        cout << "         set to maximum value (" << maxbins-1 << ") now." << endl;
        endingBin = maxbins-1;
    }
    
    //
    //   Define all tables. Depending on tableset type some
    //   will not be written but we define them all anyway.
    //
    Table tableT;
    Table tableL;
    Table tableT2;
    Table tableL2;
    Table tableVarT;
    Table tableVarL;
    Table tableLambdaT;
    Table tableLambdaL;
    Table tableTnum;
    Table tableLnum;
    
    bool logQ2=true, logW2=true, logT=false, logC=true;
    
    //
    //  Set filenames for the tables.
    //  Be as desciptive as possible. Helps when mass producing tables.
    //
    //  We create all tables and decide later what
    //  gets written and what not.
    //
    string rootfile=settings->rootfile();
    rootfile += "_" + settings->dipoleModelName();
    rootfile += "_" + settings->dipoleModelParameterSetName();
    
    ostringstream filenameT, filenameL, filenameT2,
    filenameL2, filenameVarT, filenameVarL,
    filenameLambdaT, filenameLambdaL;
    filenameT.str("");
    filenameT << rootfile << "_" << settings->vectorMesonId() << "_bin"
    << startingBin << "-" << endingBin <<"_T.root";
    filenameL.str("");
    filenameL << rootfile << "_" << settings->vectorMesonId() << "_bin"
    << startingBin << "-" << endingBin << "_L.root";
    filenameT2.str("");
    filenameT2 << rootfile << "_" << settings->vectorMesonId() << "_bin"
    << startingBin << "-" << endingBin << "_T2.root";
    filenameL2.str("");
    filenameL2 << rootfile << "_" << settings->vectorMesonId() << "_bin"
    << startingBin << "-" << endingBin << "_L2.root";
    filenameVarT.str("");
    filenameVarT << rootfile << "_" << settings->vectorMesonId() << "_bin"
    << startingBin << "-" << endingBin << "_VarT.root";
    filenameVarL.str("");
    filenameVarL << rootfile << "_" << settings->vectorMesonId() << "_bin"
    << startingBin << "-" << endingBin << "_VarL.root";
    filenameLambdaT.str("");
    filenameLambdaT << rootfile << "_" << settings->vectorMesonId() << "_bin"
    << startingBin << "-" << endingBin << "_LambdaT.root";
    filenameLambdaL.str("");
    filenameLambdaL << rootfile << "_" << settings->vectorMesonId() << "_bin"
    << startingBin << "-" << endingBin << "_LambdaL.root";
    
    (void) tableT.create(nBinQ2, Q2min, Q2max,
                         nBinW2, W2min, W2max,
                         nBinT,  tmin,  tmax,
                         logQ2, logW2, logT, logC,       // all bools
                         mean_A, transverse,
                         massA, vmPDG, model, pset,
                         filenameT.str().c_str(),
                         priority, hotspots);
    (void) tableL.create(nBinQ2, Q2min, Q2max,
                         nBinW2, W2min, W2max,
                         nBinT,  tmin,  tmax,
                         logQ2, logW2, logT, logC,      // all bools
                         mean_A, longitudinal,
                         massA, vmPDG, model, pset,
                         filenameL.str().c_str(),
                         priority, hotspots);
    (void) tableT2.create(nBinQ2, Q2min, Q2max,
                          nBinW2, W2min, W2max,
                          nBinT,  tmin,  tmax,
                          logQ2, logW2, logT, logC,      // all bools
                          mean_A2, transverse,
                          massA, vmPDG, model, pset,
                          filenameT2.str().c_str(),
                          priority, hotspots);
    (void) tableL2.create(nBinQ2, Q2min, Q2max,
                          nBinW2, W2min, W2max,
                          nBinT,  tmin,  tmax,
                          logQ2, logW2, logT, logC,      // all bools
                          mean_A2, longitudinal,
                          massA, vmPDG, model, pset,
                          filenameL2.str().c_str(),
                          priority, hotspots);
    bool contentVar = modes == 1 ? false : logC;  // if mode = 1 they will be 0 so lin is better
    (void) tableVarT.create(nBinQ2, Q2min, Q2max,
                            nBinW2, W2min, W2max,
                            nBinT,  tmin,  tmax,
                            logQ2, logW2, logT, contentVar,      // all bools
                            variance_A, transverse,
                            massA, vmPDG, model, pset,
                            filenameVarT.str().c_str(),
                            priority, hotspots);
    (void) tableVarL.create(nBinQ2, Q2min, Q2max,
                            nBinW2, W2min, W2max,
                            nBinT,  tmin,  tmax,
                            logQ2, logW2, logT, contentVar,      // all bools
                            variance_A, longitudinal,
                            massA, vmPDG, model, pset,
                            filenameVarL.str().c_str(),
                            priority, hotspots);
    double contentLambda = false; // content lin
    (void) tableLambdaT.create(nBinQ2, Q2min, Q2max,
                               nBinW2, W2min, W2max,
                               nBinT,  tmin,  tmax,
                               logQ2, logW2, logT, contentLambda,      // all bools
                               lambda_real, transverse,
                               massA, vmPDG, model, pset,
                               filenameLambdaT.str().c_str(),
                               priority, hotspots);
    (void) tableLambdaL.create(nBinQ2, Q2min, Q2max,
                               nBinW2, W2min, W2max,
                               nBinT,  tmin,  tmax,
                               logQ2, logW2, logT, contentLambda,      // all bools
                               lambda_real, longitudinal,
                               massA, vmPDG, model, pset,
                               filenameLambdaL.str().c_str(),
                               priority, hotspots);
    
    //
    // We want to save the results from the averaging even when it is not used in the tables
    // Just in order to not destroy information that was difficult to obtain.
    //
    if(modes==0){
        ostringstream filenameTnum, filenameLnum;
        filenameTnum.str("");
        filenameTnum << rootfile << "_" << settings->vectorMesonId() << "_bin"
        << startingBin << "-" << endingBin <<"numerical_T.root";
        filenameLnum.str("");
        filenameLnum << rootfile << "_" << settings->vectorMesonId() << "_bin"
        << startingBin << "-" << endingBin << "numerical_L.root";
        (void) tableTnum.create(nBinQ2, Q2min, Q2max,
                                nBinW2, W2min, W2max,
                                nBinT,  tmin,  tmax,
                                logQ2, logW2, logT, logC,       // all bools
                                mean_A, transverse,
                                massA, vmPDG, model, pset,
                                filenameT.str().c_str(),
                                priority, hotspots);
        (void) tableLnum.create(nBinQ2, Q2min, Q2max,
                                nBinW2, W2min, W2max,
                                nBinT,  tmin,  tmax,
                                logQ2, logW2, logT, logC,      // all bools
                                mean_A, longitudinal,
                                massA, vmPDG, model, pset,
                                filenameL.str().c_str(),
                                priority, hotspots);
    }
    
    cout << "\nAll tables created:" << endl;
    tableT.list();
    tableL.list();
    tableT2.list();
    tableL2.list();
    tableVarT.list();
    tableVarL.list();
    tableLambdaT.list();
    tableLambdaL.list();
    if(modes==0){
        tableTnum.list();
        tableLnum.list();
    }
    
    cout << "\nTables have " << maxbins << " bins each.\n" << endl;
    
    if (settings->useBackupFile()) {
        int ibin = settings->startingBinFromBackup();
        tableT.setAutobackup("tableT", ibin);
        tableL.setAutobackup("tableL", ibin);
        tableT2.setAutobackup("tableT2", ibin);
        tableL2.setAutobackup("tableL2", ibin);
        tableVarT.setAutobackup("tableVarT", ibin);
        tableVarL.setAutobackup("tableVarL", ibin);
        tableLambdaT.setAutobackup("tableLambdaT", ibin);
        tableLambdaL.setAutobackup("tableLambdaL", ibin);
        if(modes==0){
            tableTnum.setAutobackup("tableTnum", ibin);
            tableLnum.setAutobackup("tableLnum", ibin);
        }
        cout << "Automatic backup of tables is enabled." << endl;
    }
    else
        cout << "Automatic backup of tables is off.\n" << endl;
    
    //
    // Setup random generator for creating partial tables.
    //
    TRandom3 *random = TableGeneratorSettings::randomGenerator();
    random->SetSeed();

    //
    //   DGLAP Evolution can be speed up by using lookup tables
    //
    DglapEvolution &dglap = DglapEvolution::instance();
    dglap.generateLookupTable(1000, 1000);
    dglap.useLookupTable(true);
    
    //
    //   Create and initialize the amplitudes calculator
    //
    Amplitudes amps;
    
    //
    //   Generate the the nucleon configurations
    //
    amps.generateConfigurations();
    
    //
    //   Print out settings
    //
    cout << endl;
    cout << "Tables will be generated for:" << endl;
    cout << "\tNucleus mass A="<<massA<<endl;
    cout << "\tModes to calculate: " << modes << endl;
    cout << "\tVector Meson Id: " << vmPDG << endl;
    cout << "\tBins: " << startingBin << "-" << endingBin << endl;
    cout << "\tQ2 range: [" << Q2min << ", " << Q2max << "], " << nBinQ2 << " bins." << endl;
    cout << "\tW2 range: [" << W2min << ", " << W2max << "], " << nBinW2 << " bins." << endl;
    cout << "\t t range: [" << tmin << ", " << tmax << "], " << nBinT << " bins." << endl;
    cout << "\tDipole model: " << settings->dipoleModelName() << endl;
    cout << "\tDipole model parameter set: " << settings->dipoleModelParameterSetName() << endl;
    cout << "\tTable set mode: " << settings->tableSetTypeName() << endl;
    cout << endl;
    
    //
    //   Calculate contents and fill tables
    //
    //   Note that we fill all tables. What is written
    //   at the end is another story.
    //
    
    cout << "Start filling tables" << endl;
    time_t tableFillStart = time(0);
    
    int nShow = (endingBin - startingBin)/100;
    if(nShow==0) nShow=1;

    for (int i=startingBin; i<=endingBin; i++) {
        if (i%nShow == 0 || i == startingBin || i == endingBin)
            cout << "processing bin " << i << endl;

        if(fractionOfBinsToFill <= random->Uniform()) //Choose fractionOfBinsToFill of the bins;
            continue;

        double Q2, W2, t;
        tableT.binCenter(i, Q2, W2, t);
        double kinematicPoint[3]={t, Q2, W2};
        amps.calculate(kinematicPoint);
        
        double aT = 0;
        double aL = 0;
        double aT2 = 0;
        double aL2 = 0;
        double aVarT = 0;
        double aVarL = 0;
        double aTnum = 0;
        double aLnum = 0;
        
        //
        //  For mode = 1 only coherent is
        //  calculated and incoherent is therefore
        //  assumed to be 0.
        //  Hence total == coherent
        //
        //  Recall:
        //  coherent = A
        //  total = A2
        //  incoherent = variance = A2 - A*A
        //
        aL = amps.amplitudeL();
        aT = amps.amplitudeT();
        tableT.fill(i, aT);
        tableL.fill(i, aL);
        
        if (modes != 1) {
            aT2 = amps.amplitudeT2();
            aL2 = amps.amplitudeL2();
        }
        else {
            aT2 = aT*aT;
            aL2 = aL*aL;
        }
        if(modes==0){
            aTnum=amps.amplitudeTnum();
            aLnum=amps.amplitudeLnum();
            tableTnum.fill(i, aTnum);
            tableLnum.fill(i, aLnum);
        }
        tableT2.fill(i, aT2);
        tableL2.fill(i, aL2);
        
        if (modes != 1) {
            aVarT = aT2-aT*aT;
            aVarL = aL2-aL*aL;
        }
        tableVarT.fill(i, aVarT);
        tableVarL.fill(i, aVarL);
        
        //
        //  Calculate lambda and fill table
        //
        if (createLambdaTables) {
            double hplus, hminus;
            hplus=hminus=(W2max-W2min)/(4*1e4); //This value comes from tests of large and small W2
            hminus=min(hminus, W2-W2min);
            hplus=min(hplus, W2max-W2);
            hminus -= numeric_limits<float>::epsilon();
            hplus -= numeric_limits<float>::epsilon();
            double lambda[2]={0, 0};
            kinematicPoint[2]=W2+hplus;
            amps.calculate(kinematicPoint);
            double ampPlus[2]={0, 0};
            ampPlus[0]=amps.amplitudeT();
            ampPlus[1]=amps.amplitudeL();
            
            kinematicPoint[2]=W2-hminus;
            amps.calculate(kinematicPoint);
            double ampMinus[2]={0, 0};
            ampMinus[0]=amps.amplitudeT();
            ampMinus[1]=amps.amplitudeL();
            for (int j=0; j<2; j++) {
                // Don't calculate unless numerically viable:
                if (ampPlus[j]==0 || ampMinus[j]==0) {
                    lambda[j]=0;
                }
                else {
                    //Calculate derivate d(logA/dW2)=log(A+/A-)/(h+-h-):
                    double derivate=log(abs(ampPlus[j]/ampMinus[j]))/(hplus+hminus);
                    //  Finally calculate lambda:
                    double jacobian = (W2-protonMass2+Q2);
                    lambda[j] = jacobian*derivate;
                }
            }
            tableLambdaT.fill(i, lambda[0]);
            tableLambdaL.fill(i, lambda[1]);
        }
    }
    time_t tableFillEnd = time(0);
    
    //
    //   Report CPU time/cell
    //
    cout << endl;
    cout << "CPU time/bin: "
    <<  static_cast<double>(tableFillEnd-tableFillStart)/(endingBin-startingBin+1)
    << " s" << endl;
    cout << "Total time: " << static_cast<double>(tableFillEnd-tableFillStart)/60./60. << " h" << endl << endl;
    
    //
    //  We write out all tables.
    //
    //  Whoever runs the production can then decide
    //  later what to keep and what to delete.
    //  If the desired run mode is total_and_coherent or
    //  coherent_and_incoherent, if this is just to improve
    //  a coherent table in some phase space, or if one wants
    //  to maintain redundancy, all these factor might affect
    //  your choice.
    //
    tableT.write();
    tableL.write();
    tableT2.write();
    tableL2.write();
    tableVarT.write();
    tableVarL.write();
    if (createLambdaTables) {
        tableLambdaT.write();
        tableLambdaL.write();
    }
    if(modes==0){
        tableTnum.write();
        tableLnum.write();
    }
    cout << "All tables written" << endl;
    
    cout << "All done. Bye." << endl;
    
    return 0;
}  
