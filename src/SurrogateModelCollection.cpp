// //==============================================================================
// //  SurrogateModelCollection.cpp
// //==============================================================================
// #include "SurrogateModelCollection.h"
// #include "surrogate_model.h"   // surrogate_guarded::SurrogateModel -- see header note

// #include <algorithm>
// #include <cstdlib>
// #include <filesystem>
// #include <iostream>
// #include <sstream>
// #include <stdexcept>

// namespace fs = std::filesystem;
// using namespace std;

// namespace {

// // Maps the TQQ/LQQ/TQQG label stored in each channel file to the
// // (polarization, Fock state) pair it represents:
// //   TQQ  -> transverse,   qqbar
// //   LQQ  -> longitudinal, qqbar
// //   TQQG -> transverse,   qqbarg
// // Adjust the enumerator names here if Enumerations.h spells them
// // differently.
// void channelInfoFromLabel(const string& label, GammaPolarization& pol, FockState& fock) {
//     if (label == "TQQ") {
//         pol = GammaPolarization::transverse;
//         fock = FockState::QQ;
//     } else if (label == "LQQ") {
//         pol = GammaPolarization::longitudinal;
//         fock = FockState::QQ;
//     } else if (label == "TQQG") {
//         pol = GammaPolarization::transverse;
//         fock = FockState::QQG;
//     } else {
//         throw runtime_error("SurrogateModelCollection: unrecognized channel label '" + label + "'");
//     }
// }

// // "q0".."q3" -> 0..3
// int quarkIdFromString(const string& s) {
//     if (s.size() < 2 || s[0] != 'q') {
//         throw runtime_error("SurrogateModelCollection: unrecognized quark id '" + s + "'");
//     }
//     return stoi(s.substr(1));
// }

// // Adjust to match the DipoleModelType enumerators actually declared in
// // Enumerations.h -- only the case used by export_cpp_guarded.py (bSat)
// // needs to be correct for this update, the others are best-effort.
// string dipoleModelTypeName(DipoleModelType typ) {
//     switch (typ) {
//         case DipoleModelType::bSat:    return "bSat";
//         case DipoleModelType::bNonSat: return "bNonSat";
//         case DipoleModelType::bCGC:    return "bCGC";
//         default:
//             throw runtime_error("SurrogateModelCollection: unhandled DipoleModelType");
//     }
// }

// // Bounds are stored as float32 in the .bin file (export_cpp_guarded.py
// // packs them with struct "<ff"), so a value the exporter set as an
// // exact boundary (e.g. beta=0.001) can round to a slightly different
// // double when widened back (0.001f -> 0.0010000000474974513...).
// // Without slack, a legitimately in-range boundary point can fail a
// // strict double comparison purely from that round-trip. This mirrors
// // PHASE_SPACE_TOL's role on the Python/training side -- there to
// // absorb float rounding noise, not to loosen genuine range checks.
// bool inPhysicalRange(const surrogate_guarded::SurrogateModel& m,
//                       double Q2, double W2, double beta, double z) {
//     auto within = [](double val, double lo, double hi) {
//         const double tol = std::max(1e-6, 1e-6 * std::max(std::abs(lo), std::abs(hi)));
//         return val >= lo - tol && val <= hi + tol;
//     };
//     return within(beta, m.physicalMin(0), m.physicalMax(0)) &&
//            within(Q2,   m.physicalMin(1), m.physicalMax(1)) &&
//            within(W2,   m.physicalMin(2), m.physicalMax(2)) &&
//            within(z,    m.physicalMin(3), m.physicalMax(3));
// }

// // Among all entries matching (pol, f, quarkID), returns the one with the
// // highest priority() whose trained range actually covers the requested
// // physical point. Priority only breaks ties among models that already
// // cover the point -- it never overrides range coverage.
// //
// // If entries exist for this key but none cover the point, returns nullptr
// // with existsForKey=true (out-of-range case). If no entry exists for this
// // key at all, returns nullptr with existsForKey=false (never-exported case).
// const SurrogateModelCollection::Entry* selectBestEntry(
//     const vector<SurrogateModelCollection::Entry>& entries,
//     GammaPolarization pol, FockState f, int quarkID,
//     double Q2, double W2, double beta, double z,
//     bool& existsForKey) {

//     const SurrogateModelCollection::Entry* best = nullptr;
//     existsForKey = false;
//     for (const auto& e : entries) {
//         if (e.polarization != pol || e.fockState != f || e.quarkID != quarkID) continue;
//         existsForKey = true;
//         if (!inPhysicalRange(*e.model, Q2, W2, beta, z)) continue;
//         if (best == nullptr || e.model->priority() > best->model->priority()) {
//             best = &e;
//         }
//     }
//     return best;
// }

// } // namespace

// SurrogateModelCollection::SurrogateModelCollection() = default;
// SurrogateModelCollection::SurrogateModelCollection(const SurrogateModelCollection&) = default;
// SurrogateModelCollection& SurrogateModelCollection::operator=(const SurrogateModelCollection&) = default;
// SurrogateModelCollection::~SurrogateModelCollection() = default;

// SurrogateModelCollection::SurrogateModelCollection(int A, DipoleModelType typ, DipoleModelParameterSet set) {
//     init(A, typ, set);
// }

// string SurrogateModelCollection::directoryFor(int A, DipoleModelType typ) const {
//     const char* base = getenv("SARTRE_DIR");
//     if (base == nullptr) {
//         throw runtime_error("SurrogateModelCollection: SARTRE_DIR environment variable is not set; "
//                              "expected models under ${SARTRE_DIR}/model/<A>/<dipole model type>/");
//     }
//     ostringstream dir;
//     dir << base << "/model/" << A << "/" << dipoleModelTypeName(typ) << "/";
//     return dir.str();
// }

// bool SurrogateModelCollection::init(int A, DipoleModelType typ, DipoleModelParameterSet set) {
//     mA = A;
//     mDipoleType = typ;
//     mDipoleSet = set;
//     mEntries.clear();

//     string dir = directoryFor(A, typ);
//     if (!fs::exists(dir) || !fs::is_directory(dir)) {
//         cerr << "SurrogateModelCollection: model directory not found: " << dir << endl;
//         return false;
//     }
//     scanDirectory(dir);
//     return !mEntries.empty();
// }

// void SurrogateModelCollection::scanDirectory(const string& dir) {
//     for (const auto& dirEntry : fs::directory_iterator(dir)) {
//         if (!dirEntry.is_regular_file()) continue;
//         if (dirEntry.path().extension() != ".bin") continue;

//         shared_ptr<surrogate_guarded::SurrogateModel> model;
//         try {
//             model = make_shared<surrogate_guarded::SurrogateModel>(
//                 surrogate_guarded::SurrogateModel::load(dirEntry.path().string()));
//         } catch (const exception& ex) {
//             cerr << "SurrogateModelCollection: failed to load " << dirEntry.path()
//                  << ": " << ex.what() << endl;
//             continue;
//         }

//         if (model->nucleusA() != mA) {
//             cerr << "SurrogateModelCollection: skipping " << dirEntry.path()
//                  << " -- nucleus A=" << model->nucleusA() << " != requested A=" << mA << endl;
//             continue;
//         }

//         Entry e;
//         e.sourceFile = dirEntry.path().string();
//         try {
//             channelInfoFromLabel(model->channelLabelName(), e.polarization, e.fockState);
//             e.quarkID = quarkIdFromString(model->quarkIdName());
//         } catch (const exception& ex) {
//             cerr << "SurrogateModelCollection: skipping " << dirEntry.path() << ": " << ex.what() << endl;
//             continue;
//         }
//         e.model = model;

//         mEntries.push_back(std::move(e));
//     }
// }

// const SurrogateModelCollection::Entry* SurrogateModelCollection::findEntry(
//     GammaPolarization pol, FockState f, int quarkID) const {
//     for (const auto& e : mEntries) {
//         if (e.polarization == pol && e.fockState == f && e.quarkID == quarkID) return &e;
//     }
//     return nullptr;
// }

// bool SurrogateModelCollection::modelExists(GammaPolarization pol, FockState f, int quarkID) const {
//     return findEntry(pol, f, quarkID) != nullptr;
// }

// bool SurrogateModelCollection::available(double Q2, double W2, double beta, double z,
//                                           GammaPolarization pol, FockState f, int quarkID,
//                                           string* reason) const {
//     bool existsForKey = false;
//     const Entry* e = selectBestEntry(mEntries, pol, f, quarkID, Q2, W2, beta, z, existsForKey);

//     if (!existsForKey) {
//         if (reason) {
//             ostringstream msg;
//             msg << "no trained model for nucleus A=" << mA
//                 << ", channel (pol=" << static_cast<int>(pol)
//                 << ", fock=" << static_cast<int>(f) << ", quarkID=q" << quarkID
//                 << ") -- this TQQ/LQQ/TQQG x quark combination was never "
//                 << "exported for this nucleus + dipole model.";
//             *reason = msg.str();
//         }
//         return false;
//     }

//     if (e == nullptr) {
//         // At least one model exists for this channel, but the requested
//         // point falls outside all of their trained ranges.
//         if (reason) {
//             ostringstream msg;
//             msg << "(Q2=" << Q2 << ", W2=" << W2 << ", beta=" << beta << ", z=" << z
//                 << ") is outside the trained range of every model available for nucleus A=" << mA
//                 << ", channel (pol=" << static_cast<int>(pol)
//                 << ", fock=" << static_cast<int>(f) << ", quarkID=q" << quarkID << ").";
//             *reason = msg.str();
//         }
//         return false;
//     }

//     return true;
// }

// double SurrogateModelCollection::get(double Q2, double W2, double beta, double z,
//                                       GammaPolarization pol, FockState f, int quarkID) const {
//     string reason;
//     if (!available(Q2, W2, beta, z, pol, f, quarkID, &reason)) {
//         throw out_of_range("SurrogateModelCollection::get: " + reason);
//     }

//     bool existsForKey = false; // guaranteed true here since available() passed
//     const Entry* e = selectBestEntry(mEntries, pol, f, quarkID, Q2, W2, beta, z, existsForKey);

//     // Input order must follow the model file's own declared input_name(0..3)
//     // mapping, not be assumed positionally -- mirrors the remapping done in
//     // SurrogateCrossSection for the single-blob format.
//     double physical[4] = { beta, Q2, W2, z };
//     static const auto physicalIndexForName = [](const string& n) -> int {
//         if (n == "beta") return 0;
//         if (n == "Q2")   return 1;
//         if (n == "W2")   return 2;
//         if (n == "z")    return 3;
//         throw runtime_error("SurrogateModelCollection::get: unrecognized input column '" + n + "'");
//     };

//     float x[4];
//     for (int j = 0; j < 4; ++j) {
//         x[j] = static_cast<float>(physical[physicalIndexForName(e->model->input_name(j))]);
//     }

//     float y = 0.f;
//     e->model->eval(x, &y);
//     return static_cast<double>(y);
// }

// void SurrogateModelCollection::list(ostream& os, bool verbose) const {
//     os << "SurrogateModelCollection: A=" << mA << ", " << mEntries.size() << " channel model(s) loaded\n";
//     for (const auto& e : mEntries) {
//         os << "  " << e.model->channelName()
//            << "  label=" << e.model->channelLabelName()
//            << "  quark=" << e.model->quarkIdName()
//            << "  priority=" << e.model->priority();
//         if (verbose) {
//             os << "\n    beta in [" << e.model->physicalMin(0) << ", " << e.model->physicalMax(0) << "]"
//                << "\n    Q2   in [" << e.model->physicalMin(1) << ", " << e.model->physicalMax(1) << "]"
//                << "\n    W2   in [" << e.model->physicalMin(2) << ", " << e.model->physicalMax(2) << "]"
//                << "\n    z    in [" << e.model->physicalMin(3) << ", " << e.model->physicalMax(3) << "]"
//                << "\n    file: " << e.sourceFile;
//         }
//         os << "\n";
//     }
// }

// double SurrogateModelCollection::minQ2(GammaPolarization pol, FockState f, int quarkID) const {
//     bool found = false;
//     double result = 0.0;
//     for (const auto& e : mEntries) {
//         if (e.polarization != pol || e.fockState != f || e.quarkID != quarkID) continue;
//         double v = e.model->physicalMin(1);
//         if (!found || v < result) result = v;
//         found = true;
//     }
//     if (!found) throw runtime_error("SurrogateModelCollection::minQ2: no such channel model");
//     return result;
// }
// double SurrogateModelCollection::maxQ2(GammaPolarization pol, FockState f, int quarkID) const {
//     bool found = false;
//     double result = 0.0;
//     for (const auto& e : mEntries) {
//         if (e.polarization != pol || e.fockState != f || e.quarkID != quarkID) continue;
//         double v = e.model->physicalMax(1);
//         if (!found || v > result) result = v;
//         found = true;
//     }
//     if (!found) throw runtime_error("SurrogateModelCollection::maxQ2: no such channel model");
//     return result;
// }
// double SurrogateModelCollection::minW2(GammaPolarization pol, FockState f, int quarkID) const {
//     bool found = false;
//     double result = 0.0;
//     for (const auto& e : mEntries) {
//         if (e.polarization != pol || e.fockState != f || e.quarkID != quarkID) continue;
//         double v = e.model->physicalMin(2);
//         if (!found || v < result) result = v;
//         found = true;
//     }
//     if (!found) throw runtime_error("SurrogateModelCollection::minW2: no such channel model");
//     return result;
// }
// double SurrogateModelCollection::maxW2(GammaPolarization pol, FockState f, int quarkID) const {
//     bool found = false;
//     double result = 0.0;
//     for (const auto& e : mEntries) {
//         if (e.polarization != pol || e.fockState != f || e.quarkID != quarkID) continue;
//         double v = e.model->physicalMax(2);
//         if (!found || v > result) result = v;
//         found = true;
//     }
//     if (!found) throw runtime_error("SurrogateModelCollection::maxW2: no such channel model");
//     return result;
// }
// double SurrogateModelCollection::minBeta(GammaPolarization pol, FockState f, int quarkID) const {
//     bool found = false;
//     double result = 0.0;
//     for (const auto& e : mEntries) {
//         if (e.polarization != pol || e.fockState != f || e.quarkID != quarkID) continue;
//         double v = e.model->physicalMin(0);
//         if (!found || v < result) result = v;
//         found = true;
//     }
//     if (!found) throw runtime_error("SurrogateModelCollection::minBeta: no such channel model");
//     return result;
// }
// double SurrogateModelCollection::maxBeta(GammaPolarization pol, FockState f, int quarkID) const {
//     bool found = false;
//     double result = 0.0;
//     for (const auto& e : mEntries) {
//         if (e.polarization != pol || e.fockState != f || e.quarkID != quarkID) continue;
//         double v = e.model->physicalMax(0);
//         if (!found || v > result) result = v;
//         found = true;
//     }
//     if (!found) throw runtime_error("SurrogateModelCollection::maxBeta: no such channel model");
//     return result;
// }
// double SurrogateModelCollection::minZ(GammaPolarization pol, FockState f, int quarkID) const {
//     bool found = false;
//     double result = 0.0;
//     for (const auto& e : mEntries) {
//         if (e.polarization != pol || e.fockState != f || e.quarkID != quarkID) continue;
//         double v = e.model->physicalMin(3);
//         if (!found || v < result) result = v;
//         found = true;
//     }
//     if (!found) throw runtime_error("SurrogateModelCollection::minZ: no such channel model");
//     return result;
// }
// double SurrogateModelCollection::maxZ(GammaPolarization pol, FockState f, int quarkID) const {
//     bool found = false;
//     double result = 0.0;
//     for (const auto& e : mEntries) {
//         if (e.polarization != pol || e.fockState != f || e.quarkID != quarkID) continue;
//         double v = e.model->physicalMax(3);
//         if (!found || v > result) result = v;
//         found = true;
//     }
//     if (!found) throw runtime_error("SurrogateModelCollection::maxZ: no such channel model");
//     return result;
// }


//==============================================================================
//  SurrogateModelCollection.cpp
//==============================================================================
#include "SurrogateModelCollection.h"
#include "surrogate_model.h"   // surrogate_guarded::SurrogateModel -- see header note

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <stdexcept>

namespace fs = std::filesystem;
using namespace std;

namespace {

// Maps the TQQ/LQQ/TQQG label stored in each channel file to the
// (polarization, Fock state) pair it represents:
//   TQQ  -> transverse,   qqbar
//   LQQ  -> longitudinal, qqbar
//   TQQG -> transverse,   qqbarg
// Adjust the enumerator names here if Enumerations.h spells them
// differently.
void channelInfoFromLabel(const string& label, GammaPolarization& pol, FockState& fock) {
    if (label == "TQQ") {
        pol = GammaPolarization::transverse;
        fock = FockState::QQ;
    } else if (label == "LQQ") {
        pol = GammaPolarization::longitudinal;
        fock = FockState::QQ;
    } else if (label == "TQQG") {
        pol = GammaPolarization::transverse;
        fock = FockState::QQG;
    } else {
        throw runtime_error("SurrogateModelCollection: unrecognized channel label '" + label + "'");
    }
}

// "q0".."q3" -> 0..3
int quarkIdFromString(const string& s) {
    if (s.size() < 2 || s[0] != 'q') {
        throw runtime_error("SurrogateModelCollection: unrecognized quark id '" + s + "'");
    }
    return stoi(s.substr(1));
}

// Adjust to match the DipoleModelType enumerators actually declared in
// Enumerations.h -- only the case used by export_cpp_guarded.py (bSat)
// needs to be correct for this update, the others are best-effort.
string dipoleModelTypeName(DipoleModelType typ) {
    switch (typ) {
        case DipoleModelType::bSat:    return "bSat";
        case DipoleModelType::bNonSat: return "bNonSat";
        case DipoleModelType::bCGC:    return "bCGC";
        default:
            throw runtime_error("SurrogateModelCollection: unhandled DipoleModelType");
    }
}

// Bounds are stored as float32 in the .bin file (export_cpp_guarded.py
// packs them with struct "<ff"), so a value the exporter set as an
// exact boundary (e.g. beta=0.001) can round to a slightly different
// double when widened back (0.001f -> 0.0010000000474974513...).
// Without slack, a legitimately in-range boundary point can fail a
// strict double comparison purely from that round-trip. This mirrors
// PHASE_SPACE_TOL's role on the Python/training side -- there to
// absorb float rounding noise, not to loosen genuine range checks.
bool inPhysicalRange(const surrogate_guarded::SurrogateModel& m,
                      double Q2, double W2, double beta, double z) {
    auto within = [](double val, double lo, double hi) {
        const double tol = std::max(1e-6, 1e-6 * std::max(std::abs(lo), std::abs(hi)));
        return val >= lo - tol && val <= hi + tol;
    };
    return within(beta, m.physicalMin(0), m.physicalMax(0)) &&
           within(Q2,   m.physicalMin(1), m.physicalMax(1)) &&
           within(W2,   m.physicalMin(2), m.physicalMax(2)) &&
           within(z,    m.physicalMin(3), m.physicalMax(3));
}

// Among all entries matching (pol, f, quarkID), returns the one with the
// highest priority() whose trained range actually covers the requested
// physical point. Priority only breaks ties among models that already
// cover the point -- it never overrides range coverage.
//
// If entries exist for this key but none cover the point, returns nullptr
// with existsForKey=true (out-of-range case). If no entry exists for this
// key at all, returns nullptr with existsForKey=false (never-exported case).
const SurrogateModelCollection::Entry* selectBestEntry(
    const vector<SurrogateModelCollection::Entry>& entries,
    GammaPolarization pol, FockState f, int quarkID,
    double Q2, double W2, double beta, double z,
    bool& existsForKey) {

    const SurrogateModelCollection::Entry* best = nullptr;
    existsForKey = false;
    for (const auto& e : entries) {
        if (e.polarization != pol || e.fockState != f || e.quarkID != quarkID) continue;
        existsForKey = true;
        if (!inPhysicalRange(*e.model, Q2, W2, beta, z)) continue;
        if (best == nullptr || e.model->priority() > best->model->priority()) {
            best = &e;
        }
    }
    return best;
}

} // namespace

SurrogateModelCollection::SurrogateModelCollection() = default;
SurrogateModelCollection::SurrogateModelCollection(const SurrogateModelCollection&) = default;
SurrogateModelCollection& SurrogateModelCollection::operator=(const SurrogateModelCollection&) = default;
SurrogateModelCollection::~SurrogateModelCollection() = default;

SurrogateModelCollection::SurrogateModelCollection(int A, DipoleModelType typ, DipoleModelParameterSet set) {
    init(A, typ, set);
}

string SurrogateModelCollection::directoryFor(int A, DipoleModelType typ) const {
    const char* base = getenv("SARTRE_DIR");
    if (base == nullptr) {
        throw runtime_error("SurrogateModelCollection: SARTRE_DIR environment variable is not set; "
                             "expected models under ${SARTRE_DIR}/model/<A>/<dipole model type>/");
    }
    ostringstream dir;
    dir << base << "/model/" << A << "/" << dipoleModelTypeName(typ) << "/";
    return dir.str();
}

bool SurrogateModelCollection::init(int A, DipoleModelType typ, DipoleModelParameterSet set) {
    mA = A;
    mDipoleType = typ;
    mDipoleSet = set;
    mEntries.clear();
    cout << "SurrogateModelCollection: loading models for A=" << A
         << ", dipole type=" << dipoleModelTypeName(typ) << endl;
    string dir = directoryFor(A, typ);
    if (!fs::exists(dir) || !fs::is_directory(dir)) {
        cerr << "SurrogateModelCollection: model directory not found: " << dir << endl;
        return false;
    }
    scanDirectory(dir);
    return !mEntries.empty();
}

void SurrogateModelCollection::scanDirectory(const string& dir) {
    for (const auto& dirEntry : fs::directory_iterator(dir)) {
        if (!dirEntry.is_regular_file()) continue;
        if (dirEntry.path().extension() != ".bin") continue;

        shared_ptr<surrogate_guarded::SurrogateModel> model;
        try {
            model = make_shared<surrogate_guarded::SurrogateModel>(
                surrogate_guarded::SurrogateModel::load(dirEntry.path().string()));
        } catch (const exception& ex) {
            cerr << "SurrogateModelCollection: failed to load " << dirEntry.path()
                 << ": " << ex.what() << endl;
            continue;
        }

        if (model->nucleusA() != mA) {
            cerr << "SurrogateModelCollection: skipping " << dirEntry.path()
                 << " -- nucleus A=" << model->nucleusA() << " != requested A=" << mA << endl;
            continue;
        }

        Entry e;
        e.sourceFile = dirEntry.path().string();
        try {
            channelInfoFromLabel(model->channelLabelName(), e.polarization, e.fockState);
            e.quarkID = quarkIdFromString(model->quarkIdName());
        } catch (const exception& ex) {
            cerr << "SurrogateModelCollection: skipping " << dirEntry.path() << ": " << ex.what() << endl;
            continue;
        }
        e.model = model;

        mEntries.push_back(std::move(e));
    }
}

const SurrogateModelCollection::Entry* SurrogateModelCollection::findEntry(
    GammaPolarization pol, FockState f, int quarkID) const {
    for (const auto& e : mEntries) {
        if (e.polarization == pol && e.fockState == f && e.quarkID == quarkID) return &e;
    }
    return nullptr;
}

bool SurrogateModelCollection::modelExists(GammaPolarization pol, FockState f, int quarkID) const {
    return findEntry(pol, f, quarkID) != nullptr;
}

bool SurrogateModelCollection::available(double Q2, double W2, double beta, double z,
                                          GammaPolarization pol, FockState f, int quarkID,
                                          string* reason) const {
    bool existsForKey = false;
    const Entry* e = selectBestEntry(mEntries, pol, f, quarkID, Q2, W2, beta, z, existsForKey);

    if (!existsForKey) {
        if (reason) {
            ostringstream msg;
            msg << "no trained model for nucleus A=" << mA
                << ", channel (pol=" << static_cast<int>(pol)
                << ", fock=" << static_cast<int>(f) << ", quarkID=q" << quarkID
                << ") -- this TQQ/LQQ/TQQG x quark combination was never "
                << "exported for this nucleus + dipole model.";
            *reason = msg.str();
        }
        return false;
    }

    if (e == nullptr) {
        // At least one model exists for this channel, but the requested
        // point falls outside all of their trained ranges.
        if (reason) {
            ostringstream msg;
            msg << "(Q2=" << Q2 << ", W2=" << W2 << ", beta=" << beta << ", z=" << z
                << ") is outside the trained range of every model available for nucleus A=" << mA
                << ", channel (pol=" << static_cast<int>(pol)
                << ", fock=" << static_cast<int>(f) << ", quarkID=q" << quarkID << ").";
            *reason = msg.str();
        }
        return false;
    }

    return true;
}

double SurrogateModelCollection::get(double Q2, double W2, double beta, double z,
                                      GammaPolarization pol, FockState f, int quarkID) const {
    string reason;
    if (!available(Q2, W2, beta, z, pol, f, quarkID, &reason)) {
        throw out_of_range("SurrogateModelCollection::get: " + reason);
    }

    bool existsForKey = false; // guaranteed true here since available() passed
    const Entry* e = selectBestEntry(mEntries, pol, f, quarkID, Q2, W2, beta, z, existsForKey);

    // Input order must follow the model file's own declared input_name(0..3)
    // mapping, not be assumed positionally -- mirrors the remapping done in
    // SurrogateCrossSection for the single-blob format.
    double physical[4] = { beta, Q2, W2, z };
    static const auto physicalIndexForName = [](const string& n) -> int {
        if (n == "beta") return 0;
        if (n == "Q2")   return 1;
        if (n == "W2")   return 2;
        if (n == "z")    return 3;
        throw runtime_error("SurrogateModelCollection::get: unrecognized input column '" + n + "'");
    };

    float x[4];
    for (int j = 0; j < 4; ++j) {
        x[j] = static_cast<float>(physical[physicalIndexForName(e->model->input_name(j))]);
    }

    float y = 0.f;
    e->model->eval(x, &y);
    return static_cast<double>(y);
}

double SurrogateModelCollection::getOrZero(double Q2, double W2, double beta, double z,
                                            GammaPolarization pol, FockState f, int quarkID,
                                            bool* wasAvailable, string* reason) const {
    bool ok = available(Q2, W2, beta, z, pol, f, quarkID, reason);
    if (wasAvailable) *wasAvailable = ok;
    if (!ok) return 0.0;
    // available() already re-derives the same selection get() will make,
    // so this call cannot throw here.
    return get(Q2, W2, beta, z, pol, f, quarkID);
}

void SurrogateModelCollection::list(ostream& os, bool verbose) const {
    os << "SurrogateModelCollection: A=" << mA << ", " << mEntries.size() << " channel model(s) loaded\n";
    for (const auto& e : mEntries) {
        os << "  " << e.model->channelName()
           << "  label=" << e.model->channelLabelName()
           << "  quark=" << e.model->quarkIdName()
           << "  priority=" << e.model->priority();
        if (verbose) {
            os << "\n    beta in [" << e.model->physicalMin(0) << ", " << e.model->physicalMax(0) << "]"
               << "\n    Q2   in [" << e.model->physicalMin(1) << ", " << e.model->physicalMax(1) << "]"
               << "\n    W2   in [" << e.model->physicalMin(2) << ", " << e.model->physicalMax(2) << "]"
               << "\n    z    in [" << e.model->physicalMin(3) << ", " << e.model->physicalMax(3) << "]"
               << "\n    file: " << e.sourceFile;
        }
        os << "\n";
    }
}

double SurrogateModelCollection::minQ2(GammaPolarization pol, FockState f, int quarkID) const {
    bool found = false;
    double result = 0.0;
    for (const auto& e : mEntries) {
        if (e.polarization != pol || e.fockState != f || e.quarkID != quarkID) continue;
        double v = e.model->physicalMin(1);
        if (!found || v < result) result = v;
        found = true;
    }
    if (!found) throw runtime_error("SurrogateModelCollection::minQ2: no such channel model");
    return result;
}
double SurrogateModelCollection::maxQ2(GammaPolarization pol, FockState f, int quarkID) const {
    bool found = false;
    double result = 0.0;
    for (const auto& e : mEntries) {
        if (e.polarization != pol || e.fockState != f || e.quarkID != quarkID) continue;
        double v = e.model->physicalMax(1);
        if (!found || v > result) result = v;
        found = true;
    }
    if (!found) throw runtime_error("SurrogateModelCollection::maxQ2: no such channel model");
    return result;
}
double SurrogateModelCollection::minW2(GammaPolarization pol, FockState f, int quarkID) const {
    bool found = false;
    double result = 0.0;
    for (const auto& e : mEntries) {
        if (e.polarization != pol || e.fockState != f || e.quarkID != quarkID) continue;
        double v = e.model->physicalMin(2);
        if (!found || v < result) result = v;
        found = true;
    }
    if (!found) throw runtime_error("SurrogateModelCollection::minW2: no such channel model");
    return result;
}
double SurrogateModelCollection::maxW2(GammaPolarization pol, FockState f, int quarkID) const {
    bool found = false;
    double result = 0.0;
    for (const auto& e : mEntries) {
        if (e.polarization != pol || e.fockState != f || e.quarkID != quarkID) continue;
        double v = e.model->physicalMax(2);
        if (!found || v > result) result = v;
        found = true;
    }
    if (!found) throw runtime_error("SurrogateModelCollection::maxW2: no such channel model");
    return result;
}
double SurrogateModelCollection::minBeta(GammaPolarization pol, FockState f, int quarkID) const {
    bool found = false;
    double result = 0.0;
    for (const auto& e : mEntries) {
        if (e.polarization != pol || e.fockState != f || e.quarkID != quarkID) continue;
        double v = e.model->physicalMin(0);
        if (!found || v < result) result = v;
        found = true;
    }
    if (!found) throw runtime_error("SurrogateModelCollection::minBeta: no such channel model");
    return result;
}
double SurrogateModelCollection::maxBeta(GammaPolarization pol, FockState f, int quarkID) const {
    bool found = false;
    double result = 0.0;
    for (const auto& e : mEntries) {
        if (e.polarization != pol || e.fockState != f || e.quarkID != quarkID) continue;
        double v = e.model->physicalMax(0);
        if (!found || v > result) result = v;
        found = true;
    }
    if (!found) throw runtime_error("SurrogateModelCollection::maxBeta: no such channel model");
    return result;
}
double SurrogateModelCollection::minZ(GammaPolarization pol, FockState f, int quarkID) const {
    bool found = false;
    double result = 0.0;
    for (const auto& e : mEntries) {
        if (e.polarization != pol || e.fockState != f || e.quarkID != quarkID) continue;
        double v = e.model->physicalMin(3);
        if (!found || v < result) result = v;
        found = true;
    }
    if (!found) throw runtime_error("SurrogateModelCollection::minZ: no such channel model");
    return result;
}
double SurrogateModelCollection::maxZ(GammaPolarization pol, FockState f, int quarkID) const {
    bool found = false;
    double result = 0.0;
    for (const auto& e : mEntries) {
        if (e.polarization != pol || e.fockState != f || e.quarkID != quarkID) continue;
        double v = e.model->physicalMax(3);
        if (!found || v > result) result = v;
        found = true;
    }
    if (!found) throw runtime_error("SurrogateModelCollection::maxZ: no such channel model");
    return result;
}

void SurrogateModelCollection::clear() {
    mEntries.clear();            // drops shared_ptrs -> models destroyed
    mEntries.shrink_to_fit();    // release the vector's own storage
}

double SurrogateModelCollection::minQ2() const {
    if (mEntries.empty()) throw runtime_error("SurrogateModelCollection::minQ2: no models loaded");
    double result = mEntries.front().model->physicalMin(1);
    for (const auto& e : mEntries) {
        double v = e.model->physicalMin(1);
        if (v < result) result = v;
    }
    return result;
}
double SurrogateModelCollection::maxQ2() const {
    if (mEntries.empty()) throw runtime_error("SurrogateModelCollection::maxQ2: no models loaded");
    double result = mEntries.front().model->physicalMax(1);
    for (const auto& e : mEntries) {
        double v = e.model->physicalMax(1);
        if (v > result) result = v;
    }
    return result;
}
double SurrogateModelCollection::minW2() const {
    if (mEntries.empty()) throw runtime_error("SurrogateModelCollection::minW2  : no models loaded");
    double result = mEntries.front().model->physicalMin(2);
    for (const auto& e : mEntries) {
        double v = e.model->physicalMin(2);
        if (v < result) result = v;
    }
    return result; 
}
double SurrogateModelCollection::maxW2() const {
    if (mEntries.empty()) throw runtime_error("SurrogateModelCollection::maxW2  : no models loaded");
    double result = mEntries.front().model->physicalMax(2);
    for (const auto& e : mEntries) {
        double v = e.model->physicalMax(2);
        if (v > result) result = v;
    }
    return result;
}   

double SurrogateModelCollection::minBeta() const {
    if (mEntries.empty()) throw runtime_error("SurrogateModelCollection::minBeta: no models loaded");
    double result = mEntries.front().model->physicalMin(0);
    for (const auto& e : mEntries) {
        double v = e.model->physicalMin(0);
        if (v < result) result = v;
    }
    return result;
}
double SurrogateModelCollection::maxBeta() const {
    if (mEntries.empty()) throw runtime_error("SurrogateModelCollection::maxBeta: no models loaded");
    double result = mEntries.front().model->physicalMax(0);
    for (const auto& e : mEntries) {
        double v = e.model->physicalMax(0);
        if (v > result) result = v;
    }
    return result;
}
double SurrogateModelCollection::minZ() const {
    if (mEntries.empty()) throw runtime_error("SurrogateModelCollection::minZ   : no models loaded");
    double result = mEntries.front().model->physicalMin(3);
    for (const auto& e : mEntries) {
        double v = e.model->physicalMin(3);
        if (v < result) result = v;
    }
    return result;
}
double SurrogateModelCollection::maxZ() const {
    if (mEntries.empty()) throw runtime_error("SurrogateModelCollection::maxZ   : no models loaded");
    double result = mEntries.front().model->physicalMax(3);
    for (const auto& e : mEntries) {    
        double v = e.model->physicalMax(3);
        if (v > result) result = v;
    }                   
    return result;
}   