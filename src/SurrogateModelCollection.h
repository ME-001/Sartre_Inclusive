// #ifndef SurrogateModelCollection_h
// #define SurrogateModelCollection_h

// // SurrogateModelCollection
// //
// // Directory-based counterpart to Table/TableCollection, but for the
// // physics-guarded neural-net surrogate models produced by
// // export_cpp_guarded.py (v6: one file per TQQ/LQQ/TQQG x q0..q3 channel).
// //
// // init(A, typ, set) scans:
// //     ${SARTRE_DIR}/model/<A>/<dipole model type name>/
// // for every *.bin file, loads each one via surrogate_guarded::SurrogateModel,
// // and reads back the per-file metadata (physical beta/Q2/W2/z ranges,
// // nucleus A, dipole model type/parameter set, priority, TQQ/LQQ/TQQG
// // channel label, qN quark id) that export_cpp_guarded.py embeds in every
// // file. Nothing is parsed out of filenames -- everything comes from the
// // binary header itself, so the scan is robust to whatever naming
// // convention produced the file.
// //
// // The public query API still takes (GammaPolarization, FockState,
// // quarkID) -- the TQQ/LQQ/TQQG label stored in each file is mapped to the
// // (polarization, Fock state) pair it represents internally (TQQ ->
// // transverse+qqbar, LQQ -> longitudinal+qqbar, TQQG -> transverse+qqbarg),
// // so callers elsewhere in the codebase that already think in terms of
// // those enums don't need to change.
// //
// // NOTE: this class assumes surrogate_guarded::SurrogateModel (declared in
// // surrogate_model_guarded.h, not included here) has been extended with
// // accessors for the new metadata fields:
// //   n_inputs(), input_name(i)                             [already existed]
// //   physicalMin(i), physicalMax(i)                         [new]
// //   nucleusA(), nucleusName()                              [new]
// //   dipoleModelTypeName(), dipoleModelParameterSetName()   [new]
// //   priority()                                             [new]
// //   channelName()                                          [new -- e.g. "TQQG_q2"]
// //   channelLabelName()                                     [new -- "TQQ"/"LQQ"/"TQQG"]
// //   quarkIdName()                                          [new -- "q0".."q3"]
// //   eval(const float x[4], float* y) const                 [already existed]
// // If your existing loader doesn't have these yet, add them alongside the
// // v6 header-field parsing it needs anyway.

// #include "Enumerations.h"
// #include <iostream>
// #include <memory>
// #include <ostream>
// #include <string>
// #include <vector>

// namespace surrogate_guarded { class SurrogateModel; }

// class SurrogateModelCollection {
// public:
//     SurrogateModelCollection();
//     SurrogateModelCollection(const SurrogateModelCollection&);
//     SurrogateModelCollection(int A, DipoleModelType typ, DipoleModelParameterSet set);

//     ~SurrogateModelCollection();

//     SurrogateModelCollection& operator=(const SurrogateModelCollection&);

//     // Scans ${SARTRE_DIR}/model/<A>/<typ>/ and loads every channel model
//     // found there. Returns false (collection left empty) if the directory
//     // doesn't exist or contains no usable .bin files.
//     bool init(int A, DipoleModelType typ, DipoleModelParameterSet set);

//     // Does a model for this exact (polarization, Fock state, quark id)
//     // combination exist in the collection at all (independent of kinematics)?
//     bool modelExists(GammaPolarization pol, FockState f, int quarkID) const;

//     // Is (Q2, W2, beta, z) inside the trained physical range of that
//     // specific channel model? If not, 'reason' (when non-null) is filled
//     // with a human-readable message naming the channel and the violated
//     // bound(s) -- e.g. "... is outside the trained range of channel
//     // 'TQQG_q2' (label=TQQG, quark=q2, nucleus A=1): Q2=250 not in [1, 100];"
//     bool available(double Q2, double W2, double beta, double z,
//                    GammaPolarization pol, FockState f, int quarkID,
//                    std::string* reason = nullptr) const;

//     // Evaluates the channel model. Throws std::out_of_range (same message
//     // available() would produce) if the point is outside that channel's
//     // trained range, or std::runtime_error if no such channel model was
//     // loaded at all.
//     double get(double Q2, double W2, double beta, double z,
//                GammaPolarization pol, FockState f, int quarkID) const;

//     void list(std::ostream& = std::cout, bool = false) const;

//     double minQ2(GammaPolarization pol, FockState f, int quarkID) const;
//     double maxQ2(GammaPolarization pol, FockState f, int quarkID) const;
//     double minW2(GammaPolarization pol, FockState f, int quarkID) const;
//     double maxW2(GammaPolarization pol, FockState f, int quarkID) const;
//     double minBeta(GammaPolarization pol, FockState f, int quarkID) const;
//     double maxBeta(GammaPolarization pol, FockState f, int quarkID) const;
//     double minZ(GammaPolarization pol, FockState f, int quarkID) const;
//     double maxZ(GammaPolarization pol, FockState f, int quarkID) const;

//     int nucleusA() const { return mA; }
//     DipoleModelType dipoleModelType() const { return mDipoleType; }
//     DipoleModelParameterSet dipoleModelParameterSet() const { return mDipoleSet; }
//     std::size_t size() const { return mEntries.size(); }

// // private:
// public:
//     struct Entry {
//         std::shared_ptr<surrogate_guarded::SurrogateModel> model;
//         GammaPolarization polarization{};   // derived from the TQQ/LQQ/TQQG label
//         FockState fockState{};              // derived from the TQQ/LQQ/TQQG label
//         int quarkID = -1;                   // parsed from "q0".."q3"
//         std::string sourceFile;
//     };

//     const Entry* findEntry(GammaPolarization pol, FockState f, int quarkID) const;
//     std::string directoryFor(int A, DipoleModelType typ) const;
//     void scanDirectory(const std::string& dir);

// private:
//     int mA = 0;
//     DipoleModelType mDipoleType{};
//     DipoleModelParameterSet mDipoleSet{};
//     std::vector<Entry> mEntries;
// };

// #endif

#ifndef SurrogateModelCollection_h
#define SurrogateModelCollection_h

// SurrogateModelCollection
//
// Directory-based counterpart to Table/TableCollection, but for the
// physics-guarded neural-net surrogate models produced by
// export_cpp_guarded.py (v6: one file per TQQ/LQQ/TQQG x q0..q3 channel).
//
// init(A, typ, set) scans:
//     ${SARTRE_DIR}/model/<A>/<dipole model type name>/
// for every *.bin file, loads each one via surrogate_guarded::SurrogateModel,
// and reads back the per-file metadata (physical beta/Q2/W2/z ranges,
// nucleus A, dipole model type/parameter set, priority, TQQ/LQQ/TQQG
// channel label, qN quark id) that export_cpp_guarded.py embeds in every
// file. Nothing is parsed out of filenames -- everything comes from the
// binary header itself, so the scan is robust to whatever naming
// convention produced the file.
//
// The public query API still takes (GammaPolarization, FockState,
// quarkID) -- the TQQ/LQQ/TQQG label stored in each file is mapped to the
// (polarization, Fock state) pair it represents internally (TQQ ->
// transverse+qqbar, LQQ -> longitudinal+qqbar, TQQG -> transverse+qqbarg),
// so callers elsewhere in the codebase that already think in terms of
// those enums don't need to change.
//
// NOTE: this class assumes surrogate_guarded::SurrogateModel (declared in
// surrogate_model_guarded.h, not included here) has been extended with
// accessors for the new metadata fields:
//   n_inputs(), input_name(i)                             [already existed]
//   physicalMin(i), physicalMax(i)                         [new]
//   nucleusA(), nucleusName()                              [new]
//   dipoleModelTypeName(), dipoleModelParameterSetName()   [new]
//   priority()                                             [new]
//   channelName()                                          [new -- e.g. "TQQG_q2"]
//   channelLabelName()                                     [new -- "TQQ"/"LQQ"/"TQQG"]
//   quarkIdName()                                          [new -- "q0".."q3"]
//   eval(const float x[4], float* y) const                 [already existed]
// If your existing loader doesn't have these yet, add them alongside the
// v6 header-field parsing it needs anyway.

#include "Enumerations.h"
#include <iostream>
#include <memory>
#include <ostream>
#include <string>
#include <vector>

namespace surrogate_guarded { class SurrogateModel; }

class SurrogateModelCollection {
public:
    SurrogateModelCollection();
    SurrogateModelCollection(const SurrogateModelCollection&);
    SurrogateModelCollection(int A, DipoleModelType typ, DipoleModelParameterSet set);

    ~SurrogateModelCollection();

    SurrogateModelCollection& operator=(const SurrogateModelCollection&);

    // Scans ${SARTRE_DIR}/model/<A>/<typ>/ and loads every channel model
    // found there. Returns false (collection left empty) if the directory
    // doesn't exist or contains no usable .bin files.
    bool init(int A, DipoleModelType typ, DipoleModelParameterSet set);

    // Does a model for this exact (polarization, Fock state, quark id)
    // combination exist in the collection at all (independent of kinematics)?
    bool modelExists(GammaPolarization pol, FockState f, int quarkID) const;

    // Is (Q2, W2, beta, z) inside the trained physical range of that
    // specific channel model? If not, 'reason' (when non-null) is filled
    // with a human-readable message naming the channel and the violated
    // bound(s) -- e.g. "... is outside the trained range of channel
    // 'TQQG_q2' (label=TQQG, quark=q2, nucleus A=1): Q2=250 not in [1, 100];"
    bool available(double Q2, double W2, double beta, double z,
                   GammaPolarization pol, FockState f, int quarkID,
                   std::string* reason = nullptr) const;

    // Evaluates the channel model. Throws std::out_of_range (same message
    // available() would produce) if the point is outside that channel's
    // trained range, or std::runtime_error if no such channel model was
    // loaded at all.
    double get(double Q2, double W2, double beta, double z,
               GammaPolarization pol, FockState f, int quarkID) const;

    // Non-throwing counterpart to get(): returns 0.0 instead of throwing
    // when the point is outside every loaded model's trained range for
    // this channel, or when no model for this channel was ever loaded.
    // If 'wasAvailable' is non-null, it is set to false in that out-of-range
    // / never-exported case (true otherwise). If 'reason' is non-null, it
    // is filled with the same human-readable explanation get() would have
    // thrown, whenever wasAvailable comes back false.
    double getOrZero(double Q2, double W2, double beta, double z,
                      GammaPolarization pol, FockState f, int quarkID,
                      bool* wasAvailable = nullptr, std::string* reason = nullptr) const;

    void list(std::ostream& = std::cout, bool = false) const;

    double minQ2(GammaPolarization pol, FockState f, int quarkID) const;
    double maxQ2(GammaPolarization pol, FockState f, int quarkID) const;
    double minW2(GammaPolarization pol, FockState f, int quarkID) const;
    double maxW2(GammaPolarization pol, FockState f, int quarkID) const;
    double minBeta(GammaPolarization pol, FockState f, int quarkID) const;
    double maxBeta(GammaPolarization pol, FockState f, int quarkID) const;
    double minZ(GammaPolarization pol, FockState f, int quarkID) const;
    double maxZ(GammaPolarization pol, FockState f, int quarkID) const;

    double minQ2() const;
    double maxQ2() const;
    double minW2() const;
    double maxW2() const;
    double minBeta() const;
    double maxBeta() const;
    double minZ() const;
    double maxZ() const;

    int nucleusA() const { return mA; }
    DipoleModelType dipoleModelType() const { return mDipoleType; }
    DipoleModelParameterSet dipoleModelParameterSet() const { return mDipoleSet; }
    std::size_t size() const { return mEntries.size(); }

// private:
public:
    struct Entry {
        std::shared_ptr<surrogate_guarded::SurrogateModel> model;
        GammaPolarization polarization{};   // derived from the TQQ/LQQ/TQQG label
        FockState fockState{};              // derived from the TQQ/LQQ/TQQG label
        int quarkID = -1;                   // parsed from "q0".."q3"
        std::string sourceFile;
    };

    const Entry* findEntry(GammaPolarization pol, FockState f, int quarkID) const;
    std::string directoryFor(int A, DipoleModelType typ) const;
    void scanDirectory(const std::string& dir);

private:
    int mA = 0;
    DipoleModelType mDipoleType{};
    DipoleModelParameterSet mDipoleSet{};
    std::vector<Entry> mEntries;
    
    void clear();
};

#endif