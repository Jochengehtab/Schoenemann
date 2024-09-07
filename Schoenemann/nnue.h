#include <iostream>
#include <utility>
#include <algorithm>
#include <cstdint>
#include <vector>
#include <array>

// CRelu function
int32_t crelu(int16_t x)
{
    return std::clamp(static_cast<int32_t>(x), 0, 255);
}

class Network
{
private:
    constexpr static uint16_t ColorStride = 64 * 6;
    constexpr static uint8_t PieceStride = 64;
    constexpr static uint16_t InputSize = 768;
    constexpr static uint16_t HiddenSize = 512;
    constexpr static uint16_t OutputSize = 1;


    std::array<int16_t, InputSize * HiddenSize> FeatureWeight;
    std::array<int16_t, HiddenSize> FeatureBias;
    std::array<int16_t, HiddenSize * 2 * OutputSize> OutputWeight;
    std::array<int16_t, OutputSize> OutputBias;
    std::array<int32_t, OutputSize> Output;
    uint16_t CurrentAccumulator = 0;

public:
};