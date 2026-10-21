#ifndef InclusiveDiffractiveCrossSectionsfromModel_h
#define InclusiveDiffractiveCrossSectionsfromModel_h
// #include <torch/script.h>
//==============================================================================
//  InclusiveDiffractiveCrossSectionsFromModel.h
//
//  Combines two previously separate headers:
//
//    1) SurrogateCrossSection.h -- declares SurrogateCrossSection, which
//       loads every physics-guarded channel model for one nucleus +
//       dipole model (via SurrogateModelCollection) and evaluates all 12
//       TQQ/LQQ/TQQG x q0..q3 channels at a kinematic point in one call.
//       (Implementation formerly in SurrogateCrossSectionsGuarded.cpp.)
//
//    2) InclusiveDiffractiveCrossSectionsFromModel.h -- declares
//       InclusiveDiffractionCrossSectionsFromModel, the third leaf in the
//       InclusiveDiffractiveCrossSections hierarchy (alongside
//       ...FromTables and ...Integrals). Evaluates the SurrogateCrossSection
//       above at runtime instead of reading a pre-filled lookup table,
//       applying the same physics-guard spike filter used at table-build
//       time. (Implementation formerly in
//       InclusiveDiffractiveCrossSectionsFromSurrogate.cpp.)
//
//  IMPORTANT -- translation-unit ordering, not an oversight: whatever
//  reaches LibTorch (via SurrogateModelCollection.h, pulled in below) MUST
//  do so before any ROOT header reaches Rtypes.h, or ROOT's Rtypes.h
//  macro-defines ClassDef(name,id) before LibTorch's tree_views.h parses,
//  which breaks LibTorch's own (real, scoped) ClassDef. Any .cpp that
//  #includes this header must NOT have a ROOT header earlier in its own
//  include list -- see the identical warning at the top of
//  tableGeneratorMainSurrogate_mpi.cpp.
//
//  *** NOTE ON A DISCREPANCY THAT WAS FIXED DURING THIS MERGE ***
//  The original InclusiveDiffractiveCrossSectionsFromSurrogate.cpp
//  constructed the surrogate as
//      mSurrogate = std::make_unique<SurrogateCrossSection>(modelDir);
//  i.e. from a single std::string. But SurrogateCrossSection only ever
//  wraps a SurrogateModelCollection, and SurrogateModelCollection::init()
//  only exists as init(int A, DipoleModelType typ, DipoleModelParameterSet
//  set) -- it resolves ${SARTRE_DIR}/model/<A>/<typ>/ internally
//  (directoryFor() is private) and has no directory-based entry point.
//  Since SurrogateModelCollection is not to be changed, there is no way to
//  honor a bare modelDir string here. This merge instead changes
//  InclusiveDiffractionCrossSectionsFromModel's constructor to take
//  (A, typ, set) directly -- the same three arguments SurrogateCrossSection
//  itself takes -- and drops the modelDir string entirely. See the
//  constructor below and its .cpp definition.
//==============================================================================

#include <array>
#include <cstddef>
#include <memory>
#include <string>

#include "Enumerations.h"
#include "SurrogateModelCollection.h"          // LibTorch is reached through here
#include "InclusiveDiffractiveCrossSections.h" // base class

static const std::size_t kNumChannelsGuarded = 12;

//------------------------------------------------------------------------------
//  SurrogateCrossSection
//
//  Loads every physics-guarded channel model for one nucleus + dipole
//  model from ${SARTRE_DIR}/model/<A>/<typ>/ (via SurrogateModelCollection)
//  and evaluates all 12 TQQ/LQQ/TQQG x q0..q3 channels at a kinematic
//  point in one call. Each channel is checked against its own trained
//  (beta, Q2, W2, z) range before evaluation -- a point outside a given
//  channel's range does not prevent evaluating the other channels.
//------------------------------------------------------------------------------
class SurrogateCrossSection {
public:
    // A: nucleus mass number. typ/set: dipole model used to generate the
    // training cross sections. Throws std::runtime_error if no channel
    // models could be found/loaded for this (A, typ) combination.
    SurrogateCrossSection(int A, DipoleModelType typ, DipoleModelParameterSet set);

    // Evaluates all 12 channels at once, in the fixed kValueColumnsGuarded
    // order (TQQ_q0, LQQ_q0, TQQG_q0, ..., TQQ_q3, LQQ_q3, TQQG_q3).
    // Channels whose trained range doesn't cover this kinematic point are
    // set to NaN and a message is printed to stderr naming the exact
    // channel (Fock state / polarization / quark flavor) and which
    // bound(s) were violated; the remaining channels still evaluate.
    std::array<double, kNumChannelsGuarded> evaluate(double beta, double Q2, double W2, double z) const;

    // Single-channel access with an explicit availability check.
    bool available(double beta, double Q2, double W2, double z,
                   GammaPolarization pol, FockState f, int quarkID) const;
    double get(double beta, double Q2, double W2, double z,
               GammaPolarization pol, FockState f, int quarkID) const;

    const SurrogateModelCollection& collection() const { return mCollection; }

private:
    SurrogateModelCollection mCollection;
};

//------------------------------------------------------------------------------
//  Grid parameters defining the domain the surrogate model was trained /
//  guarded on. Must match the runcard used to build the model, since the
//  guard's probe steps (ported from tableGeneratorMainSurrogate_mpi.cpp's
//  applyPhysicsGuard()) are expressed in bin units of this grid.
//------------------------------------------------------------------------------
struct SurrogateGuardGridParams {
    int    nBinQ2 = 40, nBinz = 80, nBinW2 = 50;
    double Q2min = 1, Q2max = 100;
    double zmin  = 0.001, zmax  = 0.999;
    double W2min = 1, W2max = 320*320;
    bool   isQ2Log = true;
    bool   isZLog  = false;
    bool   isW2Log = true;
};

//------------------------------------------------------------------------------
//  InclusiveDiffractionCrossSectionsFromModel
//
//  The base class InclusiveDiffractiveCrossSections owns all the shared
//  machinery (kinematics validity checks, quark-species sampling, photon
//  flux, polarization sampling, mSettings, mQuarkIndex, ...) and drives
//  everything through the three virtuals overridden here:
//  dsigmadbetadz_T / dsigmadbetadz_L / dsigmadbetadz_QQG. This class only
//  needs to supply those three.
//------------------------------------------------------------------------------
class InclusiveDiffractionCrossSectionsFromModel : public InclusiveDiffractiveCrossSections {
public:
    // A/typ/set: passed straight through to SurrogateCrossSection, which
    // resolves ${SARTRE_DIR}/model/<A>/<typ>/ itself via
    // SurrogateModelCollection::init(A, typ, set). gridParams: the
    // (beta,Q2,W2,z) grid the model was trained/guarded on -- see
    // gridParamsFromSettings() below for the usual way to build this.
    InclusiveDiffractionCrossSectionsFromModel(int A, DipoleModelType typ, DipoleModelParameterSet set,
                                                const SurrogateGuardGridParams& gridParams);
    ~InclusiveDiffractionCrossSectionsFromModel() override;

    // Builds SurrogateGuardGridParams from TableGeneratorSettings::instance(),
    // with the exact same field mapping tableGeneratorMainSurrogate_mpi.cpp
    // uses for its GridParams (logQ2=1, logW2=1, logz=0,
    // W2 = Wmin^2 .. Wmax^2). Use this if the runcard that trained/guarded
    // the model is the currently loaded TableGeneratorSettings.
    static SurrogateGuardGridParams gridParamsFromSettings();

    // The three functions the base class drives everything through.
    // mQuarkIndex / mSettings are inherited from InclusiveDiffractiveCrossSections.
    double dsigmadbetadz_T(double beta, double Q2, double W2, double z) override;
    double dsigmadbetadz_L(double beta, double Q2, double W2, double z) override;
    double dsigmadbetadz_QQG(double beta, double Q2, double W2, double ztilde) override;

    // No lookup table backs this implementation -- base class default
    // (empty) is fine, so setTableCollection() is intentionally not
    // overridden here.

    // Diagnostics -- mirrors the "Physics guard summary" counters printed by
    // the table-generator script, accumulated over this object's lifetime.
    unsigned long nEvaluatedTotal() const { return mNProcessed; }
    unsigned long nGuardedTQQ()     const { return mNGuardedTQQ; }
    unsigned long nGuardedLQQ()     const { return mNGuardedLQQ; }
    unsigned long nGuardedTQQG()    const { return mNGuardedTQQG; }

private:
    static double getQuarkMass(unsigned int quarkIndex);

    bool   stepQ2WithBoundsCheck(double Q2, double nSteps, double& result) const;
    bool   stepZWithBoundsCheck(double z, double nSteps, double& result) const;
    bool   stepW2WithBoundsCheck(double W2, double nSteps, double& result) const;
    double stepQ2Clamped(double Q2, double nSteps) const;
    double stepZClamped(double z, double nSteps) const;
    double stepW2Clamped(double W2, double nSteps) const;

    double applyPhysicsGuard(double raw_val, double Q2, double W2, double beta, double z,
                              unsigned int quarkIndex, int tableType, bool& wasAdjusted);

    double evaluateGuarded(double beta, double Q2, double W2, double z, int tableType);

    std::unique_ptr<SurrogateCrossSection> mSurrogate;
    SurrogateGuardGridParams mGrid;

    unsigned long mNProcessed   = 0;
    unsigned long mNGuardedTQQ  = 0;
    unsigned long mNGuardedLQQ  = 0;
    unsigned long mNGuardedTQQG = 0;
};

#endif