//==============================================================================
//  InclusiveDiffractiveCrossSecitonfromModel.cpp
//
//  Combines two formerly separate translation units into one:
//    1) SurrogateCrossSectionsGuarded.cpp        -> class SurrogateCrossSection
//    2) InclusiveDiffractiveCrossSectionsFromSurrogate.cpp
//                                    -> class InclusiveDiffractionCrossSectionsFromModel
//
//  See InclusiveDiffractiveCrossSecitonfromModel.h for:
//    - the merged declarations
//    - the ROOT/LibTorch include-order warning (still applies: this header
//      must be the first thing included in any .cpp that reaches it)
//    - a note on why InclusiveDiffractionCrossSectionsFromModel's
//      constructor now takes (A, typ, set) instead of a modelDir string:
//      SurrogateModelCollection (left unmodified) only supports
//      init(A, typ, set) and resolves the model directory internally, so
//      there was no way to honor a bare directory path.
//==============================================================================

// IMPORTANT: this header (-> SurrogateModelCollection.h -> LibTorch) must be
// the first include in this translation unit, before any ROOT/Sartre header
// reaches Rtypes.h -- see the header comment for why (ClassDef macro clash).


#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>

#include "Settings.h"
#include "TableGeneratorSettings.h"

#include "InclusiveDiffractiveCrossSectionsFromModel.h"
#include "SurrogateModelCollection.h" // LibTorch is reached through here

#define PR(x) cout << #x << " = " << (x) << endl;

using namespace std;

//==============================================================================
//  SurrogateCrossSection
//  (formerly SurrogateCrossSectionsGuarded.cpp)
//==============================================================================
namespace {

struct ChannelKey {
    GammaPolarization pol;
    FockState fock;
    int quarkID;
};

// Must match VALUE_COLUMNS in the guarded training/export script exactly,
// including order. Each entry corresponds to its own model *file* (one
// Fock state + polarization + quark flavor per file, loaded by
// SurrogateModelCollection) rather than one index into a single combined
// blob.
//
//   TQQ  -> qqbar,  transverse
//   LQQ  -> qqbar,  longitudinal
//   TQQG -> qqbarg, transverse
// q0..q3 -> quark flavor index (u, d, s, c), matching QUARK_MASSES order
// in the training/export scripts.
const std::array<ChannelKey, kNumChannelsGuarded> kChannelKeys = { {
    { GammaPolarization::transverse,   FockState::QQ,  0 }, // TQQ_q0
    { GammaPolarization::longitudinal, FockState::QQ,  0 }, // LQQ_q0
    { GammaPolarization::transverse,   FockState::QQG, 0 }, // TQQG_q0
    { GammaPolarization::transverse,   FockState::QQ,  1 }, // TQQ_q1
    { GammaPolarization::longitudinal, FockState::QQ,  1 }, // LQQ_q1
    { GammaPolarization::transverse,   FockState::QQG, 1 }, // TQQG_q1
    { GammaPolarization::transverse,   FockState::QQ,  2 }, // TQQ_q2
    { GammaPolarization::longitudinal, FockState::QQ,  2 }, // LQQ_q2
    { GammaPolarization::transverse,   FockState::QQG, 2 }, // TQQG_q2
    { GammaPolarization::transverse,   FockState::QQ,  3 }, // TQQ_q3
    { GammaPolarization::longitudinal, FockState::QQ,  3 }, // LQQ_q3
    { GammaPolarization::transverse,   FockState::QQG, 3 }, // TQQG_q3
} };

} // namespace

SurrogateCrossSection::SurrogateCrossSection(int A, DipoleModelType typ, DipoleModelParameterSet set) {
    if (!mCollection.init(A, typ, set)) {
        throw runtime_error("SurrogateCrossSection: failed to load any physics-guarded channel "
                             "models for nucleus A=" + to_string(A) +
                             " -- expected files under ${SARTRE_DIR}/model/" + to_string(A) +
                             "/<dipole model type>/");
    }
    cout << "SurrogateCrossSection: loaded " << mCollection.size()
         << " physics-guarded channel model(s) for nucleus A=" << A << endl;
    mCollection.list(cout, false);

    if (mCollection.size() < kNumChannelsGuarded) {
        cerr << "SurrogateCrossSection: warning -- only " << mCollection.size() << " of "
             << kNumChannelsGuarded << " expected channels were found for A=" << A
             << "; evaluate() will report those missing channels as unavailable "
             << "(NaN) rather than fail outright." << endl;
    }
}

bool SurrogateCrossSection::available(double beta, double Q2, double W2, double z,
                                       GammaPolarization pol, FockState f, int quarkID) const {
    return mCollection.available(Q2, W2, beta, z, pol, f, quarkID, nullptr);
}

double SurrogateCrossSection::get(double beta, double Q2, double W2, double z,
                                   GammaPolarization pol, FockState f, int quarkID) const {
    return mCollection.get(Q2, W2, beta, z, pol, f, quarkID);
}

std::array<double, kNumChannelsGuarded> SurrogateCrossSection::evaluate(
    double beta, double Q2, double W2, double z) const {

    std::array<double, kNumChannelsGuarded> result{};
    for (std::size_t k = 0; k < kNumChannelsGuarded; ++k) {
        const ChannelKey& key = kChannelKeys[k];
        string reason;
        if (!mCollection.available(Q2, W2, beta, z, key.pol, key.fock, key.quarkID, &reason)) {
            // cerr << "SurrogateCrossSection::evaluate: " << reason << endl;
            result[k] = numeric_limits<double>::quiet_NaN();
            continue;
        }
        result[k] = mCollection.get(Q2, W2, beta, z, key.pol, key.fock, key.quarkID);
    }
    return result;
}

//==============================================================================
//  InclusiveDiffractionCrossSectionsFromModel
//  (formerly InclusiveDiffractiveCrossSectionsFromSurrogate.cpp)
//
//  The physics-guard logic below is a direct port of applyPhysicsGuard()
//  (and its stepQ2/stepZ/stepW2 helpers) from
//  tableGeneratorMainSurrogate_mpi.cpp, re-hosted as member functions so
//  they can read the grid parameters this object was constructed with.
//  Kept in sync deliberately: if the tunables there are ever retuned,
//  mirror the change here too, or table-generation time and runtime will
//  disagree at the danger zones.
//==============================================================================
namespace {
    // Same tunables as tableGeneratorMainSurrogate_mpi.cpp.
    constexpr double SPIKE_RATIO             = 2.5;
    constexpr double DANGER_ZONE_LIGHT_TQQG  = 1.15;
    constexpr double DANGER_ZONE_CHARM_TQQG  = 1.5;
    constexpr double EDGE_EPS                = 1e-5;
    constexpr double DENOM_EPS               = 1e-8;
    constexpr double Z_PROBE_STEPS_TQQG      = 2.0;
    constexpr double Q2_PROBE_STEPS_TQQG     = 2.0;
    constexpr double W2_PROBE_STEPS_TQQG     = 2.0;
    constexpr double Q2_AHEAD_STEPS_CHARM    = 3.0;
    constexpr double W2_AHEAD_STEPS_CHARM    = 3.0;

    // table_type indices, matching SurrogateCrossSection::evaluate()'s
    // 12-element layout: idx = quark_index*3 + table_type.
    constexpr int TABLE_TYPE_TQQ  = 0;
    constexpr int TABLE_TYPE_LQQ  = 1;
    constexpr int TABLE_TYPE_TQQG = 2;
}

InclusiveDiffractionCrossSectionsFromModel::InclusiveDiffractionCrossSectionsFromModel(
        int A, DipoleModelType typ, DipoleModelParameterSet set, const SurrogateGuardGridParams& gridParams)
    : InclusiveDiffractiveCrossSections()   // base ctor pulls settings/dipole model/etc. from EventGeneratorSettings
    , mGrid(gridParams)
{
    cout << "InclusiveDiffractionCrossSectionsFromModel: loading surrogate for nucleus A="
         << A << " ..." << endl;
    // SurrogateCrossSection -> SurrogateModelCollection::init(A, typ, set)
    // resolves ${SARTRE_DIR}/model/<A>/<typ>/ internally; there is no
    // directory-based entry point on SurrogateModelCollection, so A/typ/set
    // are passed straight through rather than a modelDir string.
    mSurrogate = std::make_unique<SurrogateCrossSection>(A, typ, set);
    cout << "InclusiveDiffractionCrossSectionsFromModel: surrogate model loaded." << endl;
}

InclusiveDiffractionCrossSectionsFromModel::~InclusiveDiffractionCrossSectionsFromModel() = default;

SurrogateGuardGridParams InclusiveDiffractionCrossSectionsFromModel::gridParamsFromSettings()
{
    TableGeneratorSettings* settings = TableGeneratorSettings::instance();

    SurrogateGuardGridParams gp;
    gp.nBinQ2 = settings->Q2bins();
    gp.nBinz  = settings->zbins();
    gp.nBinW2 = settings->W2bins();
    gp.Q2min  = settings->Q2min();
    gp.Q2max  = settings->Q2max();
    double Wmin = settings->Wmin();
    double Wmax = settings->Wmax();
    gp.W2min  = Wmin * Wmin;
    gp.W2max  = Wmax * Wmax;
    gp.zmin   = settings->zmin();
    gp.zmax   = settings->zmax();

    // Same fixed binning convention as tableGeneratorMainSurrogate_mpi.cpp's
    // main(): logbeta=0, logQ2=1, logW2=1, logz=0, logC=1.
    gp.isQ2Log = true;
    gp.isW2Log = true;
    gp.isZLog  = false;

    return gp;
}

double InclusiveDiffractionCrossSectionsFromModel::getQuarkMass(unsigned int quarkIndex)
{
    switch (quarkIndex) {
        case 0: return 0.0022; // up
        case 1: return 0.0047; // down
        case 2: return 0.096;  // strange
        case 3: return 1.27;   // charm
        default: return 0.0;
    }
}

//------------------------------------------------------------------------------
//  Bounds-checked / clamped stepping helpers -- identical logic to the free
//  functions in tableGeneratorMainSurrogate_mpi.cpp, re-hosted as members so
//  they can read mGrid.
//------------------------------------------------------------------------------
bool InclusiveDiffractionCrossSectionsFromModel::stepQ2WithBoundsCheck(double Q2, double nSteps, double& result) const
{
    if (mGrid.isQ2Log) {
        double logQ2     = std::log(Q2);
        double logQ2_min = std::log(mGrid.Q2min);
        double logQ2_max = std::log(mGrid.Q2max);
        double delta     = (logQ2_max - logQ2_min) / mGrid.nBinQ2;
        double target    = logQ2 + nSteps * delta;
        if (target < logQ2_min - EDGE_EPS || target > logQ2_max + EDGE_EPS) return false;
        result = std::exp(target);
        return true;
    } else {
        double delta  = (mGrid.Q2max - mGrid.Q2min) / mGrid.nBinQ2;
        double target = Q2 + nSteps * delta;
        if (target < mGrid.Q2min - EDGE_EPS || target > mGrid.Q2max + EDGE_EPS) return false;
        result = target;
        return true;
    }
}

bool InclusiveDiffractionCrossSectionsFromModel::stepZWithBoundsCheck(double z, double nSteps, double& result) const
{
    if (mGrid.isZLog) {
        double logz     = std::log(z);
        double logz_min = std::log(mGrid.zmin);
        double logz_max = std::log(mGrid.zmax);
        double delta    = (logz_max - logz_min) / mGrid.nBinz;
        double target   = logz + nSteps * delta;
        if (target < logz_min - EDGE_EPS || target > logz_max + EDGE_EPS) return false;
        result = std::exp(target);
        return true;
    } else {
        double delta  = (mGrid.zmax - mGrid.zmin) / mGrid.nBinz;
        double target = z + nSteps * delta;
        if (target < mGrid.zmin - EDGE_EPS || target > mGrid.zmax + EDGE_EPS) return false;
        result = target;
        return true;
    }
}

bool InclusiveDiffractionCrossSectionsFromModel::stepW2WithBoundsCheck(double W2, double nSteps, double& result) const
{
    if (mGrid.isW2Log) {
        double logW2     = std::log(W2);
        double logW2_min = std::log(mGrid.W2min);
        double logW2_max = std::log(mGrid.W2max);
        double delta     = (logW2_max - logW2_min) / mGrid.nBinW2;
        double target    = logW2 + nSteps * delta;
        if (target < logW2_min - EDGE_EPS || target > logW2_max + EDGE_EPS) return false;
        result = std::exp(target);
        return true;
    } else {
        double delta  = (mGrid.W2max - mGrid.W2min) / mGrid.nBinW2;
        double target = W2 + nSteps * delta;
        if (target < mGrid.W2min - EDGE_EPS || target > mGrid.W2max + EDGE_EPS) return false;
        result = target;
        return true;
    }
}

double InclusiveDiffractionCrossSectionsFromModel::stepQ2Clamped(double Q2, double nSteps) const
{
    if (mGrid.isQ2Log) {
        double logQ2_max = std::log(mGrid.Q2max);
        double delta     = (logQ2_max - std::log(mGrid.Q2min)) / mGrid.nBinQ2;
        double target    = std::min(std::log(Q2) + nSteps * delta, logQ2_max);
        return std::exp(target);
    } else {
        double delta = (mGrid.Q2max - mGrid.Q2min) / mGrid.nBinQ2;
        return std::min(Q2 + nSteps * delta, mGrid.Q2max);
    }
}

double InclusiveDiffractionCrossSectionsFromModel::stepZClamped(double z, double nSteps) const
{
    if (mGrid.isZLog) {
        double logz_max = std::log(mGrid.zmax);
        double delta    = (logz_max - std::log(mGrid.zmin)) / mGrid.nBinz;
        double target   = std::min(std::log(z) + nSteps * delta, logz_max);
        return std::exp(target);
    } else {
        double delta = (mGrid.zmax - mGrid.zmin) / mGrid.nBinz;
        return std::min(z + nSteps * delta, mGrid.zmax);
    }
}

double InclusiveDiffractionCrossSectionsFromModel::stepW2Clamped(double W2, double nSteps) const
{
    if (mGrid.isW2Log) {
        double logW2_max = std::log(mGrid.W2max);
        double delta     = (logW2_max - std::log(mGrid.W2min)) / mGrid.nBinW2;
        double target    = std::min(std::log(W2) + nSteps * delta, logW2_max);
        return std::exp(target);
    } else {
        double delta = (mGrid.W2max - mGrid.W2min) / mGrid.nBinW2;
        return std::min(W2 + nSteps * delta, mGrid.W2max);
    }
}

//------------------------------------------------------------------------------
//  applyPhysicsGuard -- same spike filter used at table-build time, applied
//  here on-the-fly so a direct model query gets the same protection a
//  pre-guarded lookup table would have given it.
//
//  NOTE: the original source carried a fuller "spike probe" version of this
//  function (commented out) alongside the simplified hard-threshold version
//  actually in use. Both are preserved below, unmodified, exactly as they
//  appeared in the file this was merged from.
//------------------------------------------------------------------------------
/* double InclusiveDiffractionCrossSectionsFromModel::applyPhysicsGuard(
        double raw_val, double Q2, double W2, double beta, double z,
        unsigned int quarkIndex, int tableType, bool& wasAdjusted)
{
    wasAdjusted = false;

    if (beta <= 0.0 || beta >= 1.0) {
        wasAdjusted = (raw_val != 0.0);
        return 0.0;
    }

    double mf = getQuarkMass(quarkIndex);
    double threshold_limit = 4.0 * mf * mf;
    int idx = static_cast<int>(quarkIndex) * 3 + tableType;

    // TQQG: guard applies to all quark species.
    if (tableType == TABLE_TYPE_TQQG) {
        double Mqq2 = (z / beta - 1.0) * Q2;

        if (Mqq2 < threshold_limit) {
            wasAdjusted = (raw_val != 0.0);
            return 0.0;
        }

        double nn_prediction = raw_val;
        double danger_zone_multiplier = (quarkIndex == 3) ? DANGER_ZONE_CHARM_TQQG : DANGER_ZONE_LIGHT_TQQG;

        if (Mqq2 < threshold_limit * danger_zone_multiplier) {
            bool   spikeFound    = false;
            bool   haveProbe     = false;
            double nn_probe_best = 0.0;

            auto considerProbe = [&](double nn_probe) {
                if (nn_prediction > SPIKE_RATIO * nn_probe) {
                    spikeFound = true;
                    if (!haveProbe || nn_probe < nn_probe_best) nn_probe_best = nn_probe;
                    haveProbe = true;
                }
            };

            double z_probe = stepZClamped(z, Z_PROBE_STEPS_TQQG);
            if (z_probe <= mGrid.zmax) {
                std::array<double,12> vals_probe = mSurrogate->evaluate(beta, Q2, W2, z_probe);
                considerProbe(vals_probe[idx]);
            }
            double z_probe_behind;
            if (stepZWithBoundsCheck(z, -Z_PROBE_STEPS_TQQG, z_probe_behind)) {
                std::array<double,12> vals_probe = mSurrogate->evaluate(beta, Q2, W2, z_probe_behind);
                considerProbe(vals_probe[idx]);
            }
            double Q2_probe = stepQ2Clamped(Q2, Q2_PROBE_STEPS_TQQG);
            if (Q2_probe <= mGrid.Q2max) {
                std::array<double,12> vals_probe = mSurrogate->evaluate(beta, Q2_probe, W2, z);
                considerProbe(vals_probe[idx]);
            }
            double Q2_probe_behind;
            if (stepQ2WithBoundsCheck(Q2, -Q2_PROBE_STEPS_TQQG, Q2_probe_behind)) {
                std::array<double,12> vals_probe = mSurrogate->evaluate(beta, Q2_probe_behind, W2, z);
                considerProbe(vals_probe[idx]);
            }
            double W2_probe = stepW2Clamped(W2, W2_PROBE_STEPS_TQQG);
            if (W2_probe <= mGrid.W2max) {
                std::array<double,12> vals_probe = mSurrogate->evaluate(beta, Q2, W2_probe, z);
                considerProbe(vals_probe[idx]);
            }
            double W2_probe_behind;
            if (stepW2WithBoundsCheck(W2, -W2_PROBE_STEPS_TQQG, W2_probe_behind)) {
                std::array<double,12> vals_probe = mSurrogate->evaluate(beta, Q2, W2_probe_behind, z);
                considerProbe(vals_probe[idx]);
            }

            if (spikeFound) {
                double ratio = (Mqq2 - threshold_limit) / (threshold_limit * (danger_zone_multiplier - 1.0));
                wasAdjusted = true;
                return nn_probe_best * std::clamp(ratio, 0.0, 1.0);
            }
        }
        return nn_prediction;
    }

    // TQQ / LQQ: guard applies to all quark species.
    double nn_prediction = raw_val;

    double Q2_prev;
    bool has_prev = stepQ2WithBoundsCheck(Q2, -1.0, Q2_prev);

    if (has_prev) {
        double MX2_prev = Q2_prev * (1.0 - beta) / beta;
        if (MX2_prev < threshold_limit) {
            double Q2_ahead = stepQ2Clamped(Q2, Q2_AHEAD_STEPS_CHARM);
            std::array<double,12> vals_ahead_q2 = mSurrogate->evaluate(beta, Q2_ahead, W2, z);
            double nn_ahead_q2 = vals_ahead_q2[idx];

            double W2_ahead = stepW2Clamped(W2, W2_AHEAD_STEPS_CHARM);
            std::array<double,12> vals_ahead_w2 = mSurrogate->evaluate(beta, Q2, W2_ahead, z);
            double nn_ahead_w2 = vals_ahead_w2[idx];

            bool spike_q2 = nn_prediction > nn_ahead_q2;
            bool spike_w2 = nn_prediction > nn_ahead_w2;

            if (spike_q2 || spike_w2) {
                double damping_factor = 1.0;
                double Q2_hard_thresh = (beta / (1.0 - beta)) * threshold_limit;
                if (Q2_hard_thresh > 0.0) {
                    if (mGrid.isQ2Log) {
                        double denom = std::log(Q2_ahead) - std::log(Q2_hard_thresh);
                        if (denom > DENOM_EPS)
                            damping_factor = (std::log(Q2) - std::log(Q2_hard_thresh)) / denom;
                    } else {
                        double denom = Q2_ahead - Q2_hard_thresh;
                        if (denom > DENOM_EPS)
                            damping_factor = (Q2 - Q2_hard_thresh) / denom;
                    }
                }
                damping_factor = std::clamp(damping_factor, 0.0, 1.0);

                double nn_ahead_best = spike_q2 ? nn_ahead_q2 : nn_ahead_w2;
                if (spike_w2 && (!spike_q2 || nn_ahead_w2 < nn_ahead_q2))
                    nn_ahead_best = nn_ahead_w2;

                wasAdjusted = true;
                return nn_ahead_best * damping_factor;
            }
        }
    }

    return nn_prediction;
}
*/
double InclusiveDiffractionCrossSectionsFromModel::applyPhysicsGuard(
        double raw_val, double Q2, double W2, double beta, double z,
        unsigned int quarkIndex, int tableType, bool& wasAdjusted)
{
    wasAdjusted = false;

    // Kinematic boundary check
    if (beta <= 0.0 || beta >= 1.0) {
        wasAdjusted = (raw_val != 0.0);
        return 0.0;
    }

    double mf = getQuarkMass(quarkIndex);
    double threshold_limit = 4.0 * mf * mf;

    // Hard threshold for TQQG phase space
    if (tableType == TABLE_TYPE_TQQG) {
        double Mqq2 = (z / beta - 1.0) * Q2;
        if (Mqq2 < threshold_limit) {
            wasAdjusted = (raw_val != 0.0);
            return 0.0;
        }
    }

    // For TQQ / LQQ hard threshold check
    double MX2 = Q2 * (1.0 - beta) / beta;
    if (MX2 < threshold_limit) {
        wasAdjusted = (raw_val != 0.0);
        return 0.0;
    }

    return raw_val;
}

double InclusiveDiffractionCrossSectionsFromModel::evaluateGuarded(
        double beta, double Q2, double W2, double z, int tableType)
{   
    //  // Kinematic boundary check
    // if (beta <= 0.0 || beta >= 1.0) {
    //     // wasAdjusted = (raw_val != 0.0);
    //     return 0.0;
    // }

    // double mf = getQuarkMass(mQuarkIndex);
    // double threshold_limit = 4.0 * mf * mf;

    // // Hard threshold for TQQG phase space
    // if (tableType == TABLE_TYPE_TQQG) {
    //     double Mqq2 = (z / beta - 1.0) * Q2;
    //     if (Mqq2 < threshold_limit) {
    //         // wasAdjusted = (raw_val != 0.0);
    //         return 0.0;
    //     }
    // }

    // // For TQQ / LQQ hard threshold check
    // double MX2 = Q2 * (1.0 - beta) / beta;
    // if (MX2 < threshold_limit) {
    //     // wasAdjusted = (raw_val != 0.0);
    //     return 0.0;
    // }

    std::array<double, kNumChannelsGuarded> vals = mSurrogate->evaluate(beta, Q2, W2, z);
    double raw_val = vals[mQuarkIndex * 3 + tableType];

    bool adjusted = false;
    double guarded = applyPhysicsGuard(raw_val, Q2, W2, beta, z, mQuarkIndex, tableType, adjusted);

    // ++mNProcessed;
    // if (adjusted) {
    //     if      (tableType == TABLE_TYPE_TQQ)  ++mNGuardedTQQ;
    //     else if (tableType == TABLE_TYPE_LQQ)  ++mNGuardedLQQ;
    //     else                                    ++mNGuardedTQQG;
    // }
    return guarded;
}

//------------------------------------------------------------------------------
//  Public interface -- same signatures and same phase-space checks as
//  InclusiveDiffractiveCrossSectionsFromTables::dsigmadbetadz_T/_L, backed
//  by the surrogate model + guard instead of a TableCollection lookup.
//  mQuarkIndex and mSettings come from the InclusiveDiffractiveCrossSections
//  base class.
//------------------------------------------------------------------------------
double InclusiveDiffractionCrossSectionsFromModel::dsigmadbetadz_T(double beta, double Q2, double W2, double z)
{
    double MX2 = Q2*(1-beta)/beta;
    double mf = getQuarkMass(mQuarkIndex);
    double pt2 = z*(1-z)*Q2-mf*mf;
    double sqrtArg = 1.-4.*(mf*mf)/MX2;

    if (sqrtArg<0){
        if (mSettings && mSettings->verboseLevel() > 3 && mQuarkIndex==0){
            cout<<"InclusiveDiffractionCrossSectionsFromModel::dsigmadbetadz_T: ";
            cout<<"MX2<4(mf2_pt2), return 0."<<endl;
            PR(MX2);
            PR(4*mf*mf);
        }
        return 0;
    }
    double z0 = (1.-sqrt(sqrtArg))/2.; // z limits depend on beta
    if (z<z0 or z>1-z0){
        if (mSettings && mSettings->verboseLevel() > 3 && mQuarkIndex==0){
            cout<<"InclusiveDiffractionCrossSectionsFromModel::dsigmadbetadz_T: ";
            cout<<"z="<<z<<" which is smaller than z0="<<z0<<", or larger than (1-z0)="<<1-z0<<", return 0."<<endl;
        }
        return 0;
    }
    if (pt2<0){
        if (mSettings && mSettings->verboseLevel() > 3 && mQuarkIndex==0){
            cout<<"InclusiveDiffractionCrossSectionsFromModel::dsigmadbetadz_T: ";
            cout<<"Not enough phase-space for quark transverse momenta, return 0."<<endl;
            PR(z);
        }
        return 0;
    }
    if (z*(1-z)*MX2-mf*mf<0){ // kappa2 < 0
        if (mSettings && mSettings->verboseLevel() > 3 && mQuarkIndex==0){
            cout<<"InclusiveDiffractionCrossSectionsFromModel::dsigmadbetadz_T: ";
            cout<<"Not enough phase-space for quark production, z*(1-z)*MX2-mf*mf<0, return 0."<<endl;
        }
        return 0;
    }

    return evaluateGuarded(beta, Q2, W2, z, TABLE_TYPE_TQQ);
}

double InclusiveDiffractionCrossSectionsFromModel::dsigmadbetadz_L(double beta, double Q2, double W2, double z)
{
    double MX2 = Q2*(1-beta)/beta;
    double mf =  getQuarkMass(mQuarkIndex);
    double pt2 = z*(1-z)*Q2-mf*mf;
    double sqrtArg = 1.-4.*mf*mf/MX2;

    if (sqrtArg<0){
        if (mSettings && mSettings->verboseLevel() > 3 && mQuarkIndex==0){
            cout<<"InclusiveDiffractionCrossSectionsFromModel::dsigmadbetadz_L: ";
            cout<<"There is no phase-space for z, return 0."<<endl;
            PR(MX2);
            PR(4*mf*mf);
        }
        return 0;
    }
    double z0=(1.-sqrt(sqrtArg))/2.;
    if (z<z0 or z>1-z0){ // z limits depend on beta
        if (mSettings && mSettings->verboseLevel() > 3 && mQuarkIndex==0){
            cout<<"InclusiveDiffractionCrossSectionsFromModel::dsigmadbetadz_L: ";
            cout<<"z="<<z<<" which is smaller than z0="<<z0<<", or larger than (1-z0)="<<1-z0<<", return 0."<<endl;
        }
        return 0;
    }
    if (pt2<0){
        if (mSettings && mSettings->verboseLevel() > 3 && mQuarkIndex==0){
            cout<<"InclusiveDiffractionCrossSectionsFromModel::dsigmadbetadz_L: ";
            cout<<"Not enough phase-space for quark transverse momenta, return 0."<<endl;
            PR(z);
        }
        return 0;
    }
    if (z*(1-z)*MX2-mf*mf<0){ // kappa2 < 0
        if (mSettings && mSettings->verboseLevel() > 3 && mQuarkIndex==0){
            cout<<"InclusiveDiffractionCrossSectionsFromModel::dsigmadbetadz_L: ";
            cout<<"Not enough phase-space for quark production, z*(1-z)*MX2-mf*mf<0, return 0."<<endl;
        }
        return 0;
    }

    return evaluateGuarded(beta, Q2, W2, z, TABLE_TYPE_LQQ);
}

double InclusiveDiffractionCrossSectionsFromModel::dsigmadbetadz_QQG(double beta, double Q2, double W2, double ztilde)
{
    // Mirrors the original table-reading version: no extra phase-space
    // gating here -- applyPhysicsGuard()'s Mqq2 < threshold_limit check
    // already zeroes out the kinematically forbidden region.
    return evaluateGuarded(beta, Q2, W2, ztilde, TABLE_TYPE_TQQG);
}