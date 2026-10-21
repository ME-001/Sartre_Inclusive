//====================================================================================
// Version 1
//====================================================================================
// #include "surrogate_model.h"

// #include <algorithm>
// #include <cmath>
// #include <cstring>
// #include <fstream>
// #include <stdexcept>
// #include <utility>
// #include <iterator>
// #include <string>

// namespace surrogate_guarded {

// namespace {

// struct Reader {
//     const std::vector<uint8_t>& buf;
//     std::size_t pos = 0;

//     void need(std::size_t n) const {
//         if (pos + n > buf.size())
//             throw std::runtime_error("surrogate_guarded: unexpected end of file while reading blob");
//     }

//     template <typename T>
//     T read() {
//         need(sizeof(T));
//         T v;
//         std::memcpy(&v, buf.data() + pos, sizeof(T));
//         pos += sizeof(T);
//         return v;
//     }

//     std::string readString() {
//         uint32_t len = read<uint32_t>();
//         need(len);
//         std::string s(reinterpret_cast<const char*>(buf.data() + pos), len);
//         pos += len;
//         return s;
//     }

//     std::vector<float> readFloats(std::size_t n) {
//         need(n * sizeof(float));
//         std::vector<float> v(n);
//         std::memcpy(v.data(), buf.data() + pos, n * sizeof(float));
//         pos += n * sizeof(float);
//         return v;
//     }
// };

// inline float silu(float x) { return x / (1.0f + std::exp(-x)); }

// inline float softplus(float x, float beta) {
//     const float bx = beta * x;
//     if (bx > 20.0f) return x;
//     return std::log1p(std::exp(bx)) / beta;
// }

// inline float sigmoid(float x) { return 1.0f / (1.0f + std::exp(-x)); }
// inline float relu_act(float x) { return x > 0.0f ? x : 0.0f; }
// inline float tanh_act(float x) { return std::tanh(x); }

// } // namespace

// SurrogateModel SurrogateModel::load(const std::string& path) {
//     std::ifstream f(path, std::ios::binary);
//     if (!f) throw std::runtime_error("surrogate_guarded: cannot open '" + path + "'");

//     std::vector<uint8_t> buf((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
//     Reader r{buf};

//     char magic[4];
//     r.need(4);
//     std::memcpy(magic, buf.data() + r.pos, 4);
//     r.pos += 4;
//     if (std::memcmp(magic, "SURG", 4) != 0)
//         throw std::runtime_error("surrogate_guarded: bad magic in '" + path + "'");

//     SurrogateModel model;
//     // model.mVersion = r.read<uint32_t>();
//     // if (model.mVersion != 8) {
//     //     throw std::runtime_error("surrogate_guarded: unsupported version " + std::to_string(model.mVersion) +
//     //                               " in '" + path + "' (expected version 8).");
//     // }
//     model.mVersion = r.read<uint32_t>();
//     if (model.mVersion != 8 && model.mVersion != 9) {
//         throw std::runtime_error("surrogate_guarded: unsupported version " + std::to_string(model.mVersion) +
//                                   " in '" + path + "' (expected 8 or 9).");
//     }

//     uint32_t n_in = r.read<uint32_t>();

//     model.mLogFlags.resize(n_in);
//     for (uint32_t j = 0; j < n_in; ++j) model.mLogFlags[j] = r.read<uint8_t>();

//     model.mXMin = r.readFloats(n_in);
//     model.mXMax = r.readFloats(n_in);

//     model.mInputNames.resize(n_in);
//     for (uint32_t j = 0; j < n_in; ++j) model.mInputNames[j] = r.readString();

//     for (uint32_t j = 0; j < n_in; ++j) {
//         if (model.mInputNames[j] == "beta") model.mBetaIdx = j;
//         else if (model.mInputNames[j] == "Q2") model.mQ2Idx = j;
//         else if (model.mInputNames[j] == "W2") model.mW2Idx = j;
//         else if (model.mInputNames[j] == "z") model.mZIdx = j;
//     }

//     if (model.mBetaIdx == SurrogateModel::kInvalidIndex ||
//         model.mQ2Idx   == SurrogateModel::kInvalidIndex ||
//         model.mW2Idx   == SurrogateModel::kInvalidIndex ||
//         model.mZIdx    == SurrogateModel::kInvalidIndex) {
//         throw std::runtime_error("surrogate_guarded: input columns must include 'beta', 'Q2', 'W2', and 'z'");
//     }

//     model.mPhysicalRanges.resize(n_in);
//     for (uint32_t j = 0; j < n_in; ++j) {
//         model.mPhysicalRanges[j].min_val = r.read<float>();
//         model.mPhysicalRanges[j].max_val = r.read<float>();
//     }

//     model.mNucleusA = static_cast<int>(r.read<uint32_t>());
//     model.mNucleusName = r.readString();
//     model.mDipoleType = r.readString();
//     model.mDipoleParamSet = r.readString();
//     model.mPriority = static_cast<int>(r.read<int32_t>());

//     uint32_t n_channels = r.read<uint32_t>();
//     model.mChannels.resize(n_channels);
//     for (uint32_t c = 0; c < n_channels; ++c) {
//         ChannelModel& ch = model.mChannels[c];
//         ch.name = r.readString();
//         ch.log_floor = r.read<float>();
//         ch.target_scale = r.read<float>();
//         ch.floor = r.read<float>();
//         ch.type = static_cast<ChannelType>(r.read<uint8_t>());
//         ch.quark_mass = r.read<float>();
//         ch.activation = static_cast<Activation>(r.read<uint8_t>());
//         if (static_cast<uint8_t>(ch.activation) > static_cast<uint8_t>(Activation::Tanh))
//             throw std::runtime_error("surrogate_guarded: unknown activation id in '" + path + "'");
//         ch.softplus_beta = r.read<float>();
//         ch.ceiling_enabled = r.read<uint8_t>() != 0;
//         ch.ceiling_value = r.read<float>();
//         ch.label = r.readString();
//         ch.quark_id = r.readString();
//         ch.edge_mode = r.read<uint8_t>();          // v8 -- matches pack_channel
//         ch.edge_log_eps = r.read<float>();         // v8 -- matches pack_channel

//         if (model.mVersion >= 9) ch.zero_cut = r.read<float>();   // must match pack_channel order
//         else ch.zero_cut = -1.0e30f;                               // v8 file: zero rule disabled
//         if (ch.edge_mode > 1)
//             throw std::runtime_error("surrogate_guarded: unknown edge_mode in '" + path + "'");

//         uint32_t n_layers = r.read<uint32_t>();
//         ch.layers.resize(n_layers);
//         for (uint32_t l = 0; l < n_layers; ++l) {
//             LinearLayer& layer = ch.layers[l];
//             layer.in_dim = r.read<uint32_t>();
//             layer.out_dim = r.read<uint32_t>();
//             layer.weight = r.readFloats(static_cast<std::size_t>(layer.in_dim) * layer.out_dim);
//             layer.bias = r.readFloats(layer.out_dim);
//             // layer.bias = r.readFloats(layer.out_dim);
//             layer.weightT.resize(static_cast<std::size_t>(layer.in_dim) * layer.out_dim);
//             for (uint32_t o = 0; o < layer.out_dim; ++o)
//                 for (uint32_t i = 0; i < layer.in_dim; ++i)
//                     layer.weightT[static_cast<std::size_t>(i) * layer.out_dim + o] =
//                         layer.weight[static_cast<std::size_t>(o) * layer.in_dim + i];
//         }   // loop ends here; validation below needs ALL layers read

//         if (n_layers < 2)
//             throw std::runtime_error("surrogate_guarded: need at least 2 linear layers in '" + path + "'");
//         const uint32_t expected_in = n_in + (ch.edge_mode == 1 ? 2u : 0u);
//         if (ch.layers.front().in_dim != expected_in)
//             throw std::runtime_error("surrogate_guarded: first layer in_dim=" +
//                 std::to_string(ch.layers.front().in_dim) + " but expected " +
//                 std::to_string(expected_in) + " (edge_mode=" +
//                 std::to_string(ch.edge_mode) + ") in '" + path + "'");
//         for (uint32_t l = 1; l < n_layers; ++l)
//             if (ch.layers[l].in_dim != ch.layers[l - 1].out_dim)
//                 throw std::runtime_error("surrogate_guarded: layer dimension mismatch in '" + path + "'");
//         if (ch.layers.back().out_dim != 1)
//             throw std::runtime_error("surrogate_guarded: last layer out_dim must be 1 in '" + path + "'");
//         for (const auto& L : ch.layers)
//             if (L.in_dim > 256 || L.out_dim > 256)
//                 throw std::runtime_error("surrogate_guarded: layer width > 256 not supported in '" + path + "'");
//     }

//     return model;
// }

// std::size_t SurrogateModel::index_of(const std::string& channelName) const {
//     for (std::size_t i = 0; i < mChannels.size(); ++i)
//         if (mChannels[i].name == channelName) return i;
//     return kInvalidIndex;
// }

// // float SurrogateModel::normalizeOne(std::size_t j, float physicalValue) const {
// //     const float t = mLogFlags[j] ? std::log(physicalValue) : physicalValue;
// //     return (t - mXMin[j]) / (mXMax[j] - mXMin[j] + 1e-8f);
// // }
// float SurrogateModel::normalizeOne(std::size_t j, float physicalValue) const {
//     const float t = mLogFlags[j] ? std::log(physicalValue) : physicalValue;
//     return 2.0f * (t - mXMin[j]) / (mXMax[j] - mXMin[j] + 1e-8f) - 1.0f;
// }

// std::size_t SurrogateModel::semanticIndexToColumn(int semanticIdx) const {
//     switch (semanticIdx) {
//         case 0: return mBetaIdx;
//         case 1: return mQ2Idx;
//         case 2: return mW2Idx;
//         case 3: return mZIdx;
//         default:
//             throw std::runtime_error("surrogate_guarded: semanticIdx must be 0, 1, 2, or 3");
//     }
// }

// double SurrogateModel::physicalMin(int semanticIdx) const {
//     const std::size_t j = semanticIndexToColumn(semanticIdx);
//     return static_cast<double>(mPhysicalRanges[j].min_val);
// }

// double SurrogateModel::physicalMax(int semanticIdx) const {
//     const std::size_t j = semanticIndexToColumn(semanticIdx);
//     return static_cast<double>(mPhysicalRanges[j].max_val);
// }

// const std::string& SurrogateModel::channelName() const {
//     if (mChannels.empty()) {
//         throw std::runtime_error("surrogate_guarded: no channels loaded");
//     }
//     return mChannels.front().name;
// }

// std::string SurrogateModel::channelLabelName() const {
//     if (mChannels.empty()) return "";
//     return mChannels.front().label;
// }

// std::string SurrogateModel::quarkIdName() const {
//     if (mChannels.empty()) return "";
//     return mChannels.front().quark_id;
// }

// void SurrogateModel::eval(std::size_t channelIndex, const float physical[4], float* out) const {
//     const ChannelModel& ch = mChannels.at(channelIndex);

//     constexpr std::size_t kMaxW = 256;
//     float bufA[kMaxW], bufB[kMaxW];
//     float* cur = bufA;
//     float* nxt = bufB;

//     const std::size_t n_in = mInputNames.size();
//     std::size_t n_cur = 0;
//     for (std::size_t j = 0; j < n_in; ++j) cur[n_cur++] = normalizeOne(j, physical[j]);

//     if (ch.edge_mode == 1) {
//         // Must mirror edge_features() in the training script EXACTLY.
//         // Uses RAW (un-normalized, un-logged) beta, Q2, z, in double precision.
//         const double beta  = std::max(static_cast<double>(physical[mBetaIdx]), 1e-12);
//         const double Q2    = static_cast<double>(physical[mQ2Idx]);
//         const double z     = static_cast<double>(physical[mZIdx]);
//         const double mf    = static_cast<double>(ch.quark_mass);
//         const double mf2x4 = 4.0 * mf * mf;
//         const double eps   = static_cast<double>(ch.edge_log_eps);

//         double t, s;
//         if (ch.type == ChannelType::QQG) {
//             const double zmin = beta * (1.0 + mf2x4 / Q2);
//             t = std::min(std::max((z - zmin) / std::max(1.0 - zmin, 1e-12), 0.0), 1.0);
//             s = t;
//         } else {
//             const double MX2 = Q2 * (1.0 - beta) / beta;
//             const double S   = std::min(Q2, MX2);
//             const double ze  = (1.0 - std::sqrt(std::max(1.0 - mf2x4 / S, 0.0))) / 2.0;
//             t = std::min(std::max((z - ze) / std::max(1.0 - 2.0 * ze, 1e-12), 0.0), 1.0);
//             s = 4.0 * t * (1.0 - t);
//         }
//         const double lo = std::log(eps);
//         const double f1 = 2.0 * t - 1.0;
//         const double f2 = 2.0 * (std::log(s + eps) - lo) / (std::log(1.0 + eps) - lo) - 1.0;
//         cur[n_cur++] = static_cast<float>(f1);
//         cur[n_cur++] = static_cast<float>(f2);
//     }

//     const std::size_t n_layers = ch.layers.size();
//     for (std::size_t l = 0; l < n_layers; ++l) {
//         const LinearLayer& layer = ch.layers[l];
//         const uint32_t in   = layer.in_dim;
//         const uint32_t outd = layer.out_dim;
//         const float* w = layer.weightT.data();
//         const float* b = layer.bias.data();
//         float* dst = nxt;

//         // Blocks of 8 outputs: accumulators stay in registers.
//         // Per-output summation order (bias, i=0,1,2,...) is unchanged.
//         uint32_t o0 = 0;
//         for (; o0 + 8 <= outd; o0 += 8) {
//             float acc[8];
//             for (int k = 0; k < 8; ++k) acc[k] = b[o0 + k];
//             for (uint32_t i = 0; i < in; ++i) {
//                 const float a = cur[i];
//                 const float* wr = w + static_cast<std::size_t>(i) * outd + o0;
//                 for (int k = 0; k < 8; ++k) acc[k] += wr[k] * a;
//             }
//             for (int k = 0; k < 8; ++k) dst[o0 + k] = acc[k];
//         }
//         // Tail (e.g. the final out_dim = 1 layer): plain dot product on the original rows.
//         for (; o0 < outd; ++o0) {
//             float acc = b[o0];
//             const float* wrow = layer.weight.data() + static_cast<std::size_t>(o0) * in;
//             for (uint32_t i = 0; i < in; ++i) acc += wrow[i] * cur[i];
//             dst[o0] = acc;
//         }

//         if (l + 1 != n_layers) {
//             switch (ch.activation) {
//                 case Activation::SiLU:     for (uint32_t o = 0; o < outd; ++o) dst[o] = silu(dst[o]); break;
//                 case Activation::Softplus: for (uint32_t o = 0; o < outd; ++o) dst[o] = softplus(dst[o], ch.softplus_beta); break;
//                 case Activation::ReLU:     for (uint32_t o = 0; o < outd; ++o) dst[o] = relu_act(dst[o]); break;
//                 case Activation::Tanh:     for (uint32_t o = 0; o < outd; ++o) dst[o] = tanh_act(dst[o]); break;
//                 default: throw std::runtime_error("surrogate_guarded: unknown activation id");
//             }
//         }
//         std::swap(cur, nxt);
//     }

//     // const float raw_pred  = cur[0];
//     // const float pred_norm = ch.ceiling_enabled ? (ch.ceiling_value * sigmoid(raw_pred)) : raw_pred;

//     // // Inverse transform in double: log_floor can be far below -87, where float exp() underflows.
//     // const double pred_log = static_cast<double>(pred_norm) * ch.target_scale + ch.log_floor;
//     // if (pred_log < static_cast<double>(ch.zero_cut)) { *out = 0.0f; return; }
//     // *out = static_cast<float>(std::exp(pred_log));
//     const float raw_pred = cur[0];

//     // Inverse transform in double: log_floor can be far below -87, where float exp() underflows.
//     const double pred_log = static_cast<double>(raw_pred) * ch.target_scale + ch.log_floor;
    
//     if (pred_log < static_cast<double>(ch.zero_cut)) {
//         *out = 0.0f;
//         return;
//     }
    
//     // Without the sigmoid ceiling the output is unbounded, so guard against float overflow.
//     constexpr double kMaxLog = 88.7;  // ~ log(FLT_MAX)
//     *out = static_cast<float>(std::exp(std::min(pred_log, kMaxLog)));
// }

// /*
// void SurrogateModel::eval(std::size_t channelIndex, const float physical[4], float* out) const {
//     const ChannelModel& ch = mChannels.at(channelIndex);

//     const std::size_t n_in = mInputNames.size();
//     std::vector<float> activ;
//     activ.reserve(n_in + 2);
//     for (std::size_t j = 0; j < n_in; ++j) activ.push_back(normalizeOne(j, physical[j]));

//     if (ch.edge_mode == 1) {
//         // Must mirror edge_features() in the training script EXACTLY.
//         // Uses RAW (un-normalized, un-logged) beta, Q2, z, in double precision.
//         const double beta  = std::max(static_cast<double>(physical[mBetaIdx]), 1e-12);
//         const double Q2    = static_cast<double>(physical[mQ2Idx]);
//         const double z     = static_cast<double>(physical[mZIdx]);
//         const double mf    = static_cast<double>(ch.quark_mass);
//         const double mf2x4 = 4.0 * mf * mf;
//         const double eps   = static_cast<double>(ch.edge_log_eps);

//         double t, s;
//         if (ch.type == ChannelType::QQG) {
//             const double zmin = beta * (1.0 + mf2x4 / Q2);
//             t = std::min(std::max((z - zmin) / std::max(1.0 - zmin, 1e-12), 0.0), 1.0);
//             s = t;
//         } else {
//             const double MX2 = Q2 * (1.0 - beta) / beta;
//             const double S   = std::min(Q2, MX2);
//             const double ze  = (1.0 - std::sqrt(std::max(1.0 - mf2x4 / S, 0.0))) / 2.0;
//             t = std::min(std::max((z - ze) / std::max(1.0 - 2.0 * ze, 1e-12), 0.0), 1.0);
//             s = 4.0 * t * (1.0 - t);
//         }
//         const double lo = std::log(eps);
//         const double f1 = 2.0 * t - 1.0;
//         const double f2 = 2.0 * (std::log(s + eps) - lo) / (std::log(1.0 + eps) - lo) - 1.0;
//         activ.push_back(static_cast<float>(f1));
//         activ.push_back(static_cast<float>(f2));
//     }

//     for (std::size_t l = 0; l < ch.layers.size(); ++l) {
//         const LinearLayer& layer = ch.layers[l];
//         std::vector<float> next(layer.out_dim, 0.0f);
//         for (uint32_t o = 0; o < layer.out_dim; ++o) {
//             float acc = layer.bias[o];
//             const float* wrow = &layer.weight[static_cast<std::size_t>(o) * layer.in_dim];
//             for (uint32_t i = 0; i < layer.in_dim; ++i) acc += wrow[i] * activ[i];
//             next[o] = acc;
//         }
//         if (l + 1 != ch.layers.size()) {
//             switch (ch.activation) {
//                 case Activation::SiLU:     for (auto& v : next) v = silu(v); break;
//                 case Activation::Softplus: for (auto& v : next) v = softplus(v, ch.softplus_beta); break;
//                 case Activation::ReLU:     for (auto& v : next) v = relu_act(v); break;
//                 case Activation::Tanh:     for (auto& v : next) v = tanh_act(v); break;
//                 default: throw std::runtime_error("surrogate_guarded: unknown activation id");
//             }
//         }
//         activ = std::move(next);
//     }

//     const float raw_pred  = activ[0];
//     const float pred_norm = ch.ceiling_enabled ? (ch.ceiling_value * sigmoid(raw_pred)) : raw_pred;

//     // Inverse transform in double: log_floor can be far below -87, where float exp() underflows.
//     // const double pred_log = static_cast<double>(pred_norm) * ch.target_scale + ch.log_floor;
//     // *out = static_cast<float>(std::exp(pred_log));
//     const double pred_log = static_cast<double>(pred_norm) * ch.target_scale + ch.log_floor;
//     if (pred_log < static_cast<double>(ch.zero_cut)) { *out = 0.0f; return; }
//     *out = static_cast<float>(std::exp(pred_log));
// }*/
// void SurrogateModel::eval(const float physical[4], float* out) const {
//     if (mChannels.size() != 1) {
//         throw std::runtime_error("surrogate_guarded: eval requires exactly one channel per model file");
//     }
//     eval(std::size_t{0}, physical, out);
// }

// } // namespace surrogate_guarded


//====================================================================================
// Version 3
//====================================================================================
#include "surrogate_model.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <fstream>
#include <stdexcept>
#include <utility>
#include <iterator>
#include <string>

namespace surrogate_guarded {

namespace {

struct Reader {
    const std::vector<uint8_t>& buf;
    std::size_t pos = 0;

    void need(std::size_t n) const {
        if (pos + n > buf.size())
            throw std::runtime_error("surrogate_guarded: unexpected end of file while reading blob");
    }

    template <typename T>
    T read() {
        need(sizeof(T));
        T v;
        std::memcpy(&v, buf.data() + pos, sizeof(T));
        pos += sizeof(T);
        return v;
    }

    std::string readString() {
        uint32_t len = read<uint32_t>();
        need(len);
        std::string s(reinterpret_cast<const char*>(buf.data() + pos), len);
        pos += len;
        return s;
    }

    std::vector<float> readFloats(std::size_t n) {
        need(n * sizeof(float));
        std::vector<float> v(n);
        std::memcpy(v.data(), buf.data() + pos, n * sizeof(float));
        pos += n * sizeof(float);
        return v;
    }
};

inline float silu(float x) { return x / (1.0f + std::exp(-x)); }

inline float softplus(float x, float beta) {
    const float bx = beta * x;
    if (bx > 20.0f) return x;
    return std::log1p(std::exp(bx)) / beta;
}

inline float relu_act(float x) { return x > 0.0f ? x : 0.0f; }
inline float tanh_act(float x) { return std::tanh(x); }

} // namespace

SurrogateModel SurrogateModel::load(const std::string& path) {
    std::ifstream f(path, std::ios::binary);
    if (!f) throw std::runtime_error("surrogate_guarded: cannot open '" + path + "'");

    std::vector<uint8_t> buf((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
    Reader r{buf};

    char magic[4];
    r.need(4);
    std::memcpy(magic, buf.data() + r.pos, 4);
    r.pos += 4;
    if (std::memcmp(magic, "SURG", 4) != 0)
        throw std::runtime_error("surrogate_guarded: bad magic in '" + path + "'");

    SurrogateModel model;
    model.mVersion = r.read<uint32_t>();
    if (model.mVersion < 8 || model.mVersion > 10) {
        throw std::runtime_error("surrogate_guarded: unsupported version " + std::to_string(model.mVersion) +
                                  " in '" + path + "' (expected 8, 9 or 10).");
    }

    uint32_t n_in = r.read<uint32_t>();

    model.mLogFlags.resize(n_in);
    for (uint32_t j = 0; j < n_in; ++j) model.mLogFlags[j] = r.read<uint8_t>();

    model.mXMin = r.readFloats(n_in);
    model.mXMax = r.readFloats(n_in);

    model.mInputNames.resize(n_in);
    for (uint32_t j = 0; j < n_in; ++j) model.mInputNames[j] = r.readString();

    for (uint32_t j = 0; j < n_in; ++j) {
        if (model.mInputNames[j] == "beta") model.mBetaIdx = j;
        else if (model.mInputNames[j] == "Q2") model.mQ2Idx = j;
        else if (model.mInputNames[j] == "W2") model.mW2Idx = j;
        else if (model.mInputNames[j] == "z") model.mZIdx = j;
    }

    if (model.mBetaIdx == SurrogateModel::kInvalidIndex ||
        model.mQ2Idx   == SurrogateModel::kInvalidIndex ||
        model.mW2Idx   == SurrogateModel::kInvalidIndex ||
        model.mZIdx    == SurrogateModel::kInvalidIndex) {
        throw std::runtime_error("surrogate_guarded: input columns must include 'beta', 'Q2', 'W2', and 'z'");
    }

    model.mPhysicalRanges.resize(n_in);
    for (uint32_t j = 0; j < n_in; ++j) {
        model.mPhysicalRanges[j].min_val = r.read<float>();
        model.mPhysicalRanges[j].max_val = r.read<float>();
    }

    model.mNucleusA = static_cast<int>(r.read<uint32_t>());
    model.mNucleusName = r.readString();
    model.mDipoleType = r.readString();
    model.mDipoleParamSet = r.readString();
    model.mPriority = static_cast<int>(r.read<int32_t>());

    uint32_t n_channels = r.read<uint32_t>();
    model.mChannels.resize(n_channels);
    for (uint32_t c = 0; c < n_channels; ++c) {
        ChannelModel& ch = model.mChannels[c];
        ch.name = r.readString();
        ch.log_floor = r.read<float>();
        ch.target_scale = r.read<float>();
        ch.floor = r.read<float>();
        ch.type = static_cast<ChannelType>(r.read<uint8_t>());
        ch.quark_mass = r.read<float>();
        ch.activation = static_cast<Activation>(r.read<uint8_t>());
        if (static_cast<uint8_t>(ch.activation) > static_cast<uint8_t>(Activation::Tanh))
            throw std::runtime_error("surrogate_guarded: unknown activation id in '" + path + "'");
        ch.softplus_beta = r.read<float>();
        ch.ceiling_enabled = r.read<uint8_t>() != 0;
        ch.ceiling_value = r.read<float>();
        ch.label = r.readString();
        ch.quark_id = r.readString();
        ch.edge_mode = r.read<uint8_t>();
        ch.edge_log_eps = r.read<float>();

        // Order must match pack_channel in export_cpp.py.
        ch.zero_cut = (model.mVersion >= 9)  ? r.read<float>() : -1.0e30f;   // v8: zero rule disabled
        ch.zero_eps = (model.mVersion >= 10) ? r.read<float>() : 0.0f;       // v8/v9: no eps offset
        if (ch.edge_mode > 1)
            throw std::runtime_error("surrogate_guarded: unknown edge_mode in '" + path + "'");

        uint32_t n_layers = r.read<uint32_t>();
        ch.layers.resize(n_layers);
        for (uint32_t l = 0; l < n_layers; ++l) {
            LinearLayer& layer = ch.layers[l];
            layer.in_dim = r.read<uint32_t>();
            layer.out_dim = r.read<uint32_t>();
            layer.weight = r.readFloats(static_cast<std::size_t>(layer.in_dim) * layer.out_dim);
            layer.bias = r.readFloats(layer.out_dim);
            layer.weightT.resize(static_cast<std::size_t>(layer.in_dim) * layer.out_dim);
            for (uint32_t o = 0; o < layer.out_dim; ++o)
                for (uint32_t i = 0; i < layer.in_dim; ++i)
                    layer.weightT[static_cast<std::size_t>(i) * layer.out_dim + o] =
                        layer.weight[static_cast<std::size_t>(o) * layer.in_dim + i];
        }

        if (n_layers < 2)
            throw std::runtime_error("surrogate_guarded: need at least 2 linear layers in '" + path + "'");
        const uint32_t expected_in = n_in + (ch.edge_mode == 1 ? 2u : 0u);
        if (ch.layers.front().in_dim != expected_in)
            throw std::runtime_error("surrogate_guarded: first layer in_dim=" +
                std::to_string(ch.layers.front().in_dim) + " but expected " +
                std::to_string(expected_in) + " (edge_mode=" +
                std::to_string(ch.edge_mode) + ") in '" + path + "'");
        for (uint32_t l = 1; l < n_layers; ++l)
            if (ch.layers[l].in_dim != ch.layers[l - 1].out_dim)
                throw std::runtime_error("surrogate_guarded: layer dimension mismatch in '" + path + "'");
        if (ch.layers.back().out_dim != 1)
            throw std::runtime_error("surrogate_guarded: last layer out_dim must be 1 in '" + path + "'");
        for (const auto& L : ch.layers)
            if (L.in_dim > 256 || L.out_dim > 256)
                throw std::runtime_error("surrogate_guarded: layer width > 256 not supported in '" + path + "'");
    }

    return model;
}

std::size_t SurrogateModel::index_of(const std::string& channelName) const {
    for (std::size_t i = 0; i < mChannels.size(); ++i)
        if (mChannels[i].name == channelName) return i;
    return kInvalidIndex;
}

float SurrogateModel::normalizeOne(std::size_t j, float physicalValue) const {
    const float t = mLogFlags[j] ? std::log(physicalValue) : physicalValue;
    return 2.0f * (t - mXMin[j]) / (mXMax[j] - mXMin[j] + 1e-8f) - 1.0f;
}

std::size_t SurrogateModel::semanticIndexToColumn(int semanticIdx) const {
    switch (semanticIdx) {
        case 0: return mBetaIdx;
        case 1: return mQ2Idx;
        case 2: return mW2Idx;
        case 3: return mZIdx;
        default:
            throw std::runtime_error("surrogate_guarded: semanticIdx must be 0, 1, 2, or 3");
    }
}

double SurrogateModel::physicalMin(int semanticIdx) const {
    const std::size_t j = semanticIndexToColumn(semanticIdx);
    return static_cast<double>(mPhysicalRanges[j].min_val);
}

double SurrogateModel::physicalMax(int semanticIdx) const {
    const std::size_t j = semanticIndexToColumn(semanticIdx);
    return static_cast<double>(mPhysicalRanges[j].max_val);
}

const std::string& SurrogateModel::channelName() const {
    if (mChannels.empty()) {
        throw std::runtime_error("surrogate_guarded: no channels loaded");
    }
    return mChannels.front().name;
}

std::string SurrogateModel::channelLabelName() const {
    if (mChannels.empty()) return "";
    return mChannels.front().label;
}

std::string SurrogateModel::quarkIdName() const {
    if (mChannels.empty()) return "";
    return mChannels.front().quark_id;
}

void SurrogateModel::eval(std::size_t channelIndex, const float physical[4], float* out) const {
    const ChannelModel& ch = mChannels.at(channelIndex);

    constexpr std::size_t kMaxW = 256;
    float bufA[kMaxW], bufB[kMaxW];
    float* cur = bufA;
    float* nxt = bufB;

    const std::size_t n_in = mInputNames.size();
    std::size_t n_cur = 0;
    for (std::size_t j = 0; j < n_in; ++j) cur[n_cur++] = normalizeOne(j, physical[j]);

    if (ch.edge_mode == 1) {
        // Must mirror edge_features() in the training script EXACTLY.
        // Uses RAW (un-normalized, un-logged) beta, Q2, z, in double precision.
        const double beta  = std::max(static_cast<double>(physical[mBetaIdx]), 1e-12);
        const double Q2    = static_cast<double>(physical[mQ2Idx]);
        const double z     = static_cast<double>(physical[mZIdx]);
        const double mf    = static_cast<double>(ch.quark_mass);
        const double mf2x4 = 4.0 * mf * mf;
        const double eps   = static_cast<double>(ch.edge_log_eps);

        double t, s;
        if (ch.type == ChannelType::QQG) {
            const double zmin = beta * (1.0 + mf2x4 / Q2);
            t = std::min(std::max((z - zmin) / std::max(1.0 - zmin, 1e-12), 0.0), 1.0);
            s = t;
        } else {
            const double MX2 = Q2 * (1.0 - beta) / beta;
            const double S   = std::min(Q2, MX2);
            const double ze  = (1.0 - std::sqrt(std::max(1.0 - mf2x4 / S, 0.0))) / 2.0;
            t = std::min(std::max((z - ze) / std::max(1.0 - 2.0 * ze, 1e-12), 0.0), 1.0);
            s = 4.0 * t * (1.0 - t);
        }
        const double lo = std::log(eps);
        const double f1 = 2.0 * t - 1.0;
        const double f2 = 2.0 * (std::log(s + eps) - lo) / (std::log(1.0 + eps) - lo) - 1.0;
        cur[n_cur++] = static_cast<float>(f1);
        cur[n_cur++] = static_cast<float>(f2);
    }

    const std::size_t n_layers = ch.layers.size();
    for (std::size_t l = 0; l < n_layers; ++l) {
        const LinearLayer& layer = ch.layers[l];
        const uint32_t in   = layer.in_dim;
        const uint32_t outd = layer.out_dim;
        const float* w = layer.weightT.data();
        const float* b = layer.bias.data();
        float* dst = nxt;

        // Blocks of 8 outputs: accumulators stay in registers.
        // Per-output summation order (bias, i=0,1,2,...) is unchanged.
        uint32_t o0 = 0;
        for (; o0 + 8 <= outd; o0 += 8) {
            float acc[8];
            for (int k = 0; k < 8; ++k) acc[k] = b[o0 + k];
            for (uint32_t i = 0; i < in; ++i) {
                const float a = cur[i];
                const float* wr = w + static_cast<std::size_t>(i) * outd + o0;
                for (int k = 0; k < 8; ++k) acc[k] += wr[k] * a;
            }
            for (int k = 0; k < 8; ++k) dst[o0 + k] = acc[k];
        }
        // Tail (e.g. the final out_dim = 1 layer): plain dot product on the original rows.
        for (; o0 < outd; ++o0) {
            float acc = b[o0];
            const float* wrow = layer.weight.data() + static_cast<std::size_t>(o0) * in;
            for (uint32_t i = 0; i < in; ++i) acc += wrow[i] * cur[i];
            dst[o0] = acc;
        }

        if (l + 1 != n_layers) {
            switch (ch.activation) {
                case Activation::SiLU:     for (uint32_t o = 0; o < outd; ++o) dst[o] = silu(dst[o]); break;
                case Activation::Softplus: for (uint32_t o = 0; o < outd; ++o) dst[o] = softplus(dst[o], ch.softplus_beta); break;
                case Activation::ReLU:     for (uint32_t o = 0; o < outd; ++o) dst[o] = relu_act(dst[o]); break;
                case Activation::Tanh:     for (uint32_t o = 0; o < outd; ++o) dst[o] = tanh_act(dst[o]); break;
                default: throw std::runtime_error("surrogate_guarded: unknown activation id");
            }
        }
        std::swap(cur, nxt);
    }

    // Linear head: out = net(x). Inverse transform in double (log_floor can be far below -87).
    const double pred_log = static_cast<double>(cur[0]) * ch.target_scale + ch.log_floor;

    // Zero rows were trained at ln(zero_eps): map them back to exactly 0.
    if (pred_log < static_cast<double>(ch.zero_cut)) {
        *out = 0.0f;
        return;
    }

    // The head is unbounded, so guard against float overflow.
    constexpr double kMaxLog = 88.7;  // ~ log(FLT_MAX)
    // Target was ln(y + zero_eps): remove the offset, never return a negative value.
    const double y = std::exp(std::min(pred_log, kMaxLog)) - static_cast<double>(ch.zero_eps);
    *out = static_cast<float>(std::max(y, 0.0));
}

void SurrogateModel::eval(const float physical[4], float* out) const {
    if (mChannels.size() != 1) {
        throw std::runtime_error("surrogate_guarded: eval requires exactly one channel per model file");
    }
    eval(std::size_t{0}, physical, out);
}

} // namespace surrogate_guarded