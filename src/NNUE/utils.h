#pragma once

#include <array>

#include "nnueconstants.h"

class util
{
public:
    static inline std::int32_t screlu(int input)
    {
        const std::int32_t clipped = std::clamp<std::int32_t>(static_cast<std::int32_t>(input), 0, QA);
        return clipped * clipped;
    }

    static inline void addAll(
        std::array<std::int16_t, hiddenSize> &us,
        std::array<std::int16_t, hiddenSize> &them,
        const std::array<std::int16_t, inputHiddenSize> &outputBias,
        const std::uint32_t usOffset,
        const std::uint32_t themOffset)
    {
        for (std::uint16_t i = 0; i < hiddenSize; i++)
        {
            us[i] += outputBias[usOffset + i];
        }
        for (std::uint16_t i = 0; i < hiddenSize; i++)
        {
            them[i] += outputBias[themOffset + i];
        }
    }
    static inline void subAll(
        std::array<std::int16_t, hiddenSize> &us,
        std::array<std::int16_t, hiddenSize> &them,
        const std::array<std::int16_t, inputHiddenSize> &outputBias,
        const std::uint32_t usOffset,
        const std::uint32_t themOffset)
    {
        // Subtract the outputBias from the input arrays:
        for (std::uint16_t i = 0; i < hiddenSize; i++)
        {
            us[i] -= outputBias[usOffset + i];
        }
        for (std::uint16_t i = 0; i < hiddenSize; i++)
        {
            them[i] -= outputBias[themOffset + i];
        }
    }

    static int forward(
        const std::array<std::int16_t, hiddenSize> &us,
        const std::array<std::int16_t, hiddenSize> &them,
        const std::array<std::array<std::int16_t, hiddenSize * 2>, outputSize> &outputWeight,
        const std::array<std::int16_t, outputSize> &outputBias,
        const short bucket)
    {
        int eval = 0;
        for (std::uint16_t i = 0; i < hiddenSize; i++)
        {
           eval += screlu(us[i]) * outputWeight[bucket][i] +
                   screlu(them[i]) * outputWeight[bucket][i + hiddenSize];
        }
        eval /= QA;
        eval += outputBias[bucket];
        eval *= scale;
        eval /= (QA * QB);
        return eval;
    }
};