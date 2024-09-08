#pragma once

#include <array>

constexpr static uint16_t ColorStride = 64 * 6;
constexpr static uint8_t PieceStride = 64;

constexpr static uint16_t inputSize = 768;
constexpr static uint16_t hiddenSize = 512;
constexpr static uint16_t outputSize = 1;
constexpr static uint16_t stackSize = 512;
constexpr static uint16_t scale = 400;

constexpr static uint16_t QA = 255;
constexpr static uint16_t QB = 64;

class PerspectiveAccumulator
{
public:
    std::array<std::int16_t, hiddenSize> white;
    std::array<std::int16_t, hiddenSize> black;

    PerspectiveAccumulator()
    {
        zeroAccumulator();
    }

    inline void loadBias(std::array<std::int16_t, hiddenSize> &bias)
    {
        std::copy(std::begin(bias), std::end(bias), std::begin(white));
        std::copy(std::begin(bias), std::end(bias), std::begin(black));
    }

    inline void zeroAccumulator()
    {
        std::fill(std::begin(white), std::end(white), 0);
        std::fill(std::begin(black), std::end(black), 0);
    }
};
