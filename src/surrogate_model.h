//====================================================================================
// Version 1
//====================================================================================
// #ifndef SURROGATE_MODEL_H
// #define SURROGATE_MODEL_H

// #include <cstdint>
// #include <string>
// #include <vector>
// #include <cstddef>

// namespace surrogate_guarded {

// enum class ChannelType : uint8_t {
// QQ = 0,
// QQG = 1
// };

// enum class Activation : uint8_t {
// SiLU     = 0,
// Softplus = 1,
// ReLU     = 2,
// Tanh     = 3
// };

// struct LinearLayer {
// uint32_t in_dim = 0;
// uint32_t out_dim = 0;
// std::vector<float> weight;
// std::vector<float> bias;
// std::vector<float> weightT;   // (in_dim, out_dim) row-major: weightT[i*out_dim + o]
// };

// struct ChannelModel {
// std::string name;
// float log_floor = 0.0f;
// float target_scale = 0.0f;
// float floor = 0.0f;
// ChannelType type = ChannelType::QQ;
// float quark_mass = 0.0f;
// Activation activation = Activation::SiLU;
//     // Activation activation = Activation::Softplus;
// float softplus_beta = 1.0f;
// bool ceiling_enabled = false;
// float ceiling_value = 1.25f;
//     // std::string label;
//     // std::string quark_id;
//     // std::vector<LinearLayer> layers;

// std::string label;
// std::string quark_id;
// uint8_t edge_mode = 0;       // v8: 0 = none, 1 = edge_t/edge_logs appended
// float edge_log_eps = 0.0f;   // v8: regulator in ln(s + eps), used if edge_mode == 1
// float zero_cut = -1.0e30f;   // pred_log < zero_cut -> output exactly 0 (v9+)
// std::vector<LinearLayer> layers;
// };

// struct PhysicalRange {
// float min_val = 0.0f;
// float max_val = 0.0f;
// };

// class SurrogateModel {
// public:
// static constexpr std::size_t kInvalidIndex = static_cast<std::size_t>(-1);

// static SurrogateModel load(const std::string& path);

//     // Accessors for SurrogateModelCollection
// std::size_t index_of(const std::string& channelName) const;
// double physicalMin(int semanticIdx) const;
// double physicalMax(int semanticIdx) const;

// int nucleusA() const { return mNucleusA; }
// std::string nucleusName() const { return mNucleusName; }
// std::string dipoleModelTypeName() const { return mDipoleType; }
// std::string dipoleModelParameterSetName() const { return mDipoleParamSet; }
// int priority() const { return mPriority; }

// const std::string& channelName() const;
// std::string channelLabelName() const;
// std::string quarkIdName() const;

//     // Evaluation
// void eval(std::size_t channelIndex, const float physical[4], float* out) const;
// void eval(const float physical[4], float* out) const;

// std::size_t n_inputs() const { return mInputNames.size(); }
// const std::string& input_name(std::size_t j) const { return mInputNames.at(j); }

// private:
// std::size_t semanticIndexToColumn(int semanticIdx) const;
// float normalizeOne(std::size_t j, float physicalValue) const;

// uint32_t mVersion = 0;
// std::vector<uint8_t> mLogFlags;
// std::vector<float> mXMin;
// std::vector<float> mXMax;
// std::vector<std::string> mInputNames;
// std::vector<PhysicalRange> mPhysicalRanges;

// std::size_t mBetaIdx = kInvalidIndex;
// std::size_t mQ2Idx   = kInvalidIndex;
// std::size_t mW2Idx   = kInvalidIndex;
// std::size_t mZIdx    = kInvalidIndex;

// int mNucleusA = 0;
// std::string mNucleusName;
// std::string mDipoleType;
// std::string mDipoleParamSet;
// int mPriority = 0;

// std::vector<ChannelModel> mChannels;
// };

// } // namespace surrogate_guarded

// #endif // SURROGATE_MODEL_H

//====================================================================================
// Version 3
//====================================================================================
#ifndef SURROGATE_MODEL_H
#define SURROGATE_MODEL_H

#include <cstdint>
#include <string>
#include <vector>
#include <cstddef>

namespace surrogate_guarded {

enum class ChannelType : uint8_t {
    QQ = 0,
    QQG = 1
};

enum class Activation : uint8_t {
    SiLU     = 0,
    Softplus = 1,
    ReLU     = 2,
    Tanh     = 3
};

struct LinearLayer {
    uint32_t in_dim = 0;
    uint32_t out_dim = 0;
    std::vector<float> weight;
    std::vector<float> bias;
    std::vector<float> weightT;   // (in_dim, out_dim) row-major: weightT[i*out_dim + o]
};

struct ChannelModel {
    std::string name;
    float log_floor = 0.0f;
    float target_scale = 0.0f;
    float floor = 0.0f;
    ChannelType type = ChannelType::QQ;
    float quark_mass = 0.0f;
    Activation activation = Activation::SiLU;
    float softplus_beta = 1.0f;
    bool ceiling_enabled = false;
    float ceiling_value = 1.25f;

    std::string label;
    std::string quark_id;
    uint8_t edge_mode = 0;       // v8: 0 = none, 1 = edge_t/edge_logs appended
    float edge_log_eps = 0.0f;   // v8: regulator in ln(s + eps), used if edge_mode == 1
    float zero_cut = -1.0e30f;   // pred_log < zero_cut -> output exactly 0 (v9+)
    float zero_eps = 0.0f;       // v10+: training target was ln(y + zero_eps); y = max(exp(pred_log) - zero_eps, 0)
    std::vector<LinearLayer> layers;
};

struct PhysicalRange {
    float min_val = 0.0f;
    float max_val = 0.0f;
};

class SurrogateModel {
public:
    static constexpr std::size_t kInvalidIndex = static_cast<std::size_t>(-1);

    static SurrogateModel load(const std::string& path);

    // Accessors for SurrogateModelCollection
    std::size_t index_of(const std::string& channelName) const;
    double physicalMin(int semanticIdx) const;
    double physicalMax(int semanticIdx) const;

    int nucleusA() const { return mNucleusA; }
    std::string nucleusName() const { return mNucleusName; }
    std::string dipoleModelTypeName() const { return mDipoleType; }
    std::string dipoleModelParameterSetName() const { return mDipoleParamSet; }
    int priority() const { return mPriority; }

    const std::string& channelName() const;
    std::string channelLabelName() const;
    std::string quarkIdName() const;

    // Evaluation
    void eval(std::size_t channelIndex, const float physical[4], float* out) const;
    void eval(const float physical[4], float* out) const;

    std::size_t n_inputs() const { return mInputNames.size(); }
    const std::string& input_name(std::size_t j) const { return mInputNames.at(j); }

private:
    std::size_t semanticIndexToColumn(int semanticIdx) const;
    float normalizeOne(std::size_t j, float physicalValue) const;

    uint32_t mVersion = 0;
    std::vector<uint8_t> mLogFlags;
    std::vector<float> mXMin;
    std::vector<float> mXMax;
    std::vector<std::string> mInputNames;
    std::vector<PhysicalRange> mPhysicalRanges;

    std::size_t mBetaIdx = kInvalidIndex;
    std::size_t mQ2Idx   = kInvalidIndex;
    std::size_t mW2Idx   = kInvalidIndex;
    std::size_t mZIdx    = kInvalidIndex;

    int mNucleusA = 0;
    std::string mNucleusName;
    std::string mDipoleType;
    std::string mDipoleParamSet;
    int mPriority = 0;

    std::vector<ChannelModel> mChannels;
};

} // namespace surrogate_guarded

#endif // SURROGATE_MODEL_H