
// ==================================================================================================================
// sf3.cpp
//================================================================================================================
#include <iostream>
#include <vector>
#include <cmath>
#include "TFile.h"
#include "TString.h"
#include "TCanvas.h"
#include "TH1D.h"
#include "TDirectory.h"
#include "SartreInclusiveDiffraction.h"
#include "DipoleModelParameters.h"
#include "Settings.h"
#include "TROOT.h"
#include "InclusiveDiffractiveCrossSections.h"
#include "DglapEvolution.h"
#include "Kinematics.h"
#include "TableCollection.h"
#include "Table.h"
#include "InclusiveDiffractiveCrossSectionsFromTables.h"

#include "Math/GaussLegendreIntegrator.h"
#include "Math/WrappedTF1.h"

using namespace std;

// ---------------------------------------------------------------------
// Globals needed by the TF1 wrapper functions below.
// ---------------------------------------------------------------------
InclusiveDiffractiveCrossSections* myICS = 0;

double mFT(double* var, double* par)
{
    return myICS->dsigmadbetadz_T(par[0], par[1], par[2], var[0]);
}
double mFL(double* var, double* par)
{
    return myICS->dsigmadbetadz_L(par[0], par[1], par[2], var[0]);
}
double mFQQG(double* var, double* par)
{
    return myICS->dsigmadbetadz_QQG(par[0], par[1], par[2], var[0]);
}

struct SFResult { double T = 0, L = 0, QQG = 0, Tot = 0; };

// ---------------------------------------------------------------------
// Sum over 4 quark flavors for given (beta, Q2, x).
// ---------------------------------------------------------------------
SFResult computeSF(double beta, double Q2, double x,
                    TF1& fT, TF1& fL, TF1& fQQG,
                    ROOT::Math::GaussLegendreIntegrator& giT,
                    ROOT::Math::GaussLegendreIntegrator& giL,
                    ROOT::Math::GaussLegendreIntegrator& giQQG)
{
    SFResult r;
    double W2 = Kinematics::W2(Q2, x);
    double prefactor = beta * Q2 / (4 * M_PI * M_PI * alpha_em) / 1e7 / hbarc2;

    for (int quark = 0; quark < 4; quark++) {
        myICS->setQuarkIndex(quark);

        fT.SetParameter(0, beta); fL.SetParameter(0, beta); fQQG.SetParameter(0, beta);
        fT.SetParameter(1, Q2); fL.SetParameter(1, Q2); fQQG.SetParameter(1, Q2);
        fT.SetParameter(2, W2); fL.SetParameter(2, W2); fQQG.SetParameter(2, W2);

        r.T += giT.Integral(0.001, 0.999);
        r.L += giL.Integral(0.001, 0.999);
        r.QQG += giQQG.Integral(0.001, 0.999);
    }
    r.T *= prefactor;
    r.L *= prefactor;
    r.QQG *= prefactor;
    r.Tot = r.T + r.L + r.QQG;
    return r;
}

int main(int argc, char* argv[])
{
    if (argc < 2) {
        cout << "Usage: " << argv[0] << " runcard" << endl;
        return 2;
    }
    string runcard = argv[1];

    SartreInclusiveDiffraction sartre;
    if (!sartre.init(runcard)) {
        cerr << "Initialization of sartre failed." << endl;
        return 1;
    }
    EventGeneratorSettings* settings = sartre.runSettings();
    settings->list();

    string mode, tableBin;
    myICS = sartre.mCrossSection;
    bool table = sartre.mUseCrossSectionTables;
    if (table) {
        TableCollection* tc = sartre.mTableCollection;
        myICS = new InclusiveDiffractionCrossSectionsFromTables(tc);
        mode = "table"; tableBin = "table";
    } else {
        myICS = new InclusiveDiffractiveCrossSectionsIntegrals();
        mode = "theory"; tableBin = "";
    }
    int a = sartre.GetA();

    TF1 fT("fT", mFT, 0.001, 0.999, 3);
    TF1 fL("fL", mFL, 0.001, 0.999, 3);
    TF1 fQQG("fQQG", mFQQG, 0.001, 0.999, 3);
    ROOT::Math::WrappedTF1 wfT(fT), wfL(fL), wfQQG(fQQG);
    ROOT::Math::GaussLegendreIntegrator giT(1024), giL(1024), giQQG(1024);
    giT.SetFunction(wfT); giT.SetAbsTolerance(0.); giT.SetRelTolerance(1e-16);
    giL.SetFunction(wfL); giL.SetAbsTolerance(0.); giL.SetRelTolerance(1e-16);
    giQQG.SetFunction(wfQQG); giQQG.SetAbsTolerance(0.); giQQG.SetRelTolerance(1e-16);

    // ---------------- Scan Config ----------------
    const vector<double> Q2values = {5.0, 10.0, 50.0};
    const vector<double> betaFixedValues = {0.01, 0.1, 0.5, 0.8};
    const double xpomFixed = 0.001;

    const int betaBins = 80;
    const double betaMin = 0.001, betaMax = 0.999;

    // xpom scan bounds updated here
    const int xpomBins = 40;
    const double xpomMin = 1e-4, xpomMax = 0.01; // <--- Set xpomMax to 0.01
    
    // Logarithmic binning edges for xpom histogram
    vector<double> xpomEdges(xpomBins + 1);
    {
        double lo = log10(xpomMin), hi = log10(xpomMax);
        for (int i = 0; i <= xpomBins; i++)
            xpomEdges[i] = pow(10.0, lo + i * (hi - lo) / xpomBins);
    }

    TFile* hfile = new TFile(Form("SF_plots_%s_A%d%s.root", mode.c_str(), a,
                                   tableBin.empty() ? "" : ("_" + tableBin).c_str()),
                              "RECREATE");
    TDirectory* dirVsBeta = hfile->mkdir("vs_beta");
    TDirectory* dirVsXpom = hfile->mkdir("vs_xpom");

    TString pdfName = Form("SF_plots_%s_A%d%s.pdf", mode.c_str(), a,
                            tableBin.empty() ? "" : ("_" + tableBin).c_str());
    TCanvas* c1 = new TCanvas("c1", "SF plot", 900, 700);
    bool firstPage = true;

    const char* compNames[4] = {"F_{T}", "F_{L}", "F_{QQG}", "F_{tot}"};
    const char* compTags[4] = {"T", "L", "QQG", "Tot"};

    // =======================================================================
    // 1. SF vs beta Histograms (xpom = 0.001 fixed)
    // =======================================================================
    for (size_t qIdx = 0; qIdx < Q2values.size(); ++qIdx) {
        double Q2 = Q2values[qIdx];

        dirVsBeta->cd();
        TH1D* hT   = new TH1D(Form("h_SF_T_vsBeta_Q2_%g", Q2),   Form("%s vs #beta (x_{IP} = %.3f, Q^{2} = %g GeV^{2});#beta;%s", compNames[0], xpomFixed, Q2, compNames[0]), betaBins, betaMin, betaMax);
        TH1D* hL   = new TH1D(Form("h_SF_L_vsBeta_Q2_%g", Q2),   Form("%s vs #beta (x_{IP} = %.3f, Q^{2} = %g GeV^{2});#beta;%s", compNames[1], xpomFixed, Q2, compNames[1]), betaBins, betaMin, betaMax);
        TH1D* hQQG = new TH1D(Form("h_SF_QQG_vsBeta_Q2_%g", Q2), Form("%s vs #beta (x_{IP} = %.3f, Q^{2} = %g GeV^{2});#beta;%s", compNames[2], xpomFixed, Q2, compNames[2]), betaBins, betaMin, betaMax);
        TH1D* hTot = new TH1D(Form("h_SF_Tot_vsBeta_Q2_%g", Q2), Form("%s vs #beta (x_{IP} = %.3f, Q^{2} = %g GeV^{2});#beta;%s", compNames[3], xpomFixed, Q2, compNames[3]), betaBins, betaMin, betaMax);

        TH1D* histList[4] = {hT, hL, hQQG, hTot};

        for (int i = 1; i <= betaBins; i++) {
            double beta = hT->GetBinCenter(i);
            double x = beta * xpomFixed;
            SFResult r = computeSF(beta, Q2, x, fT, fL, fQQG, giT, giL, giQQG);

            hT->SetBinContent(i, r.T);
            hL->SetBinContent(i, r.L);
            hQQG->SetBinContent(i, r.QQG);
            hTot->SetBinContent(i, r.Tot);

            cout << "vs-beta Q2=" << Q2 << " bin " << i << "/" << betaBins
                 << " beta=" << beta << " Tot=" << r.Tot << endl;
        }

        for (int comp = 0; comp < 4; comp++) {
            TH1D* h = histList[comp];
            h->Write();

            // Render to PDF output
            c1->Clear();
            c1->cd();
            c1->SetLogx(false);
            c1->SetLogy(true);
            c1->SetGrid();
            h->SetLineWidth(2);
            h->Draw("HIST");

            c1->Print(pdfName + (firstPage ? "(" : ""));
            firstPage = false;
        }
    }

    // =======================================================================
    // 2. SF vs xpom Histograms
    // =======================================================================
    for (size_t bi = 0; bi < betaFixedValues.size(); bi++) {
        double betaFixed = betaFixedValues[bi];

        for (size_t qIdx = 0; qIdx < Q2values.size(); ++qIdx) {
            double Q2 = Q2values[qIdx];

            dirVsXpom->cd();
            TH1D* hT   = new TH1D(Form("h_SF_T_vsXpom_beta_%.2f_Q2_%g", betaFixed, Q2),   Form("%s vs x_{IP} (#beta = %.2f, Q^{2} = %g GeV^{2});x_{IP};%s", compNames[0], betaFixed, Q2, compNames[0]), xpomBins, xpomEdges.data());
            TH1D* hL   = new TH1D(Form("h_SF_L_vsXpom_beta_%.2f_Q2_%g", betaFixed, Q2),   Form("%s vs x_{IP} (#beta = %.2f, Q^{2} = %g GeV^{2});x_{IP};%s", compNames[1], betaFixed, Q2, compNames[1]), xpomBins, xpomEdges.data());
            TH1D* hQQG = new TH1D(Form("h_SF_QQG_vsXpom_beta_%.2f_Q2_%g", betaFixed, Q2), Form("%s vs x_{IP} (#beta = %.2f, Q^{2} = %g GeV^{2});x_{IP};%s", compNames[2], betaFixed, Q2, compNames[2]), xpomBins, xpomEdges.data());
            TH1D* hTot = new TH1D(Form("h_SF_Tot_vsXpom_beta_%.2f_Q2_%g", betaFixed, Q2), Form("%s vs x_{IP} (#beta = %.2f, Q^{2} = %g GeV^{2});x_{IP};%s", compNames[3], betaFixed, Q2, compNames[3]), xpomBins, xpomEdges.data());

            TH1D* histList[4] = {hT, hL, hQQG, hTot};

            for (int i = 1; i <= xpomBins; i++) {
                double xpom = hT->GetBinCenter(i);
                double x = betaFixed * xpom;
                SFResult r = computeSF(betaFixed, Q2, x, fT, fL, fQQG, giT, giL, giQQG);

                hT->SetBinContent(i, r.T);
                hL->SetBinContent(i, r.L);
                hQQG->SetBinContent(i, r.QQG);
                hTot->SetBinContent(i, r.Tot);

                cout << "vs-xpom beta=" << betaFixed << " Q2=" << Q2 << " bin " << i << "/" << xpomBins
                     << " xpom=" << xpom << " Tot=" << r.Tot << endl;
            }

            for (int comp = 0; comp < 4; comp++) {
                TH1D* h = histList[comp];
                h->Write();

                // Render to PDF output
                c1->Clear();
                c1->cd();
                c1->SetLogx(true);
                c1->SetLogy(true);
                c1->SetGrid();
                h->SetLineWidth(2);
                h->Draw("HIST");

                bool isLast = (bi == betaFixedValues.size() - 1) && (qIdx == Q2values.size() - 1) && (comp == 3);
                c1->Print(pdfName + (isLast ? ")" : ""));
            }
        }
    }

    hfile->Write();
    hfile->Close();

    cout << "\nWrote histogram plots to PDF file: " << pdfName << endl;
    cout << "Wrote TH1D objects to ROOT file: " << hfile->GetName() << endl;

    return 0;
}