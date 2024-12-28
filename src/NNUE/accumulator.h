#pragma once

#include <array>

#include "nnueconstants.h"

class accumulator
{
public:
    std::array<std::int16_t, hiddenSize> white;
    std::array<std::int16_t, hiddenSize> black;

    accumulator()
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
