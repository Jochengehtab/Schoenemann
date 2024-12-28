#pragma once

#include <array>

#include "nnueconstants.h"

class utilitys
{
public:

    static inline std::int32_t crelu(int input)
    {
        const std::int32_t clipped = std::clamp<std::int32_t>(static_cast<std::int32_t>(input), 0, QA);
        return clipped * clipped;
    }

    static inline void addAll(
        std::array<std::int16_t, hiddenSize> &firstAccumulator,
        std::array<std::int16_t, hiddenSize> &secondAccumulator,
        const std::array<std::int16_t, inputHidden> &bias,
        const std::uint32_t firstDelta,
        const std::uint32_t secondDelta)
    {
        for (std::uint16_t i = 0; i < hiddenSize; i++)
        {
            firstAccumulator[i] += bias[firstDelta + i];
        }
        for (std::uint16_t i = 0; i < hiddenSize; i++)
        {
            secondAccumulator[i] += bias[secondDelta + i];
        }
    }
    static inline void subAll(
        std::array<std::int16_t, hiddenSize> &firstAccumulator,
        std::array<std::int16_t, hiddenSize> &secondAccumulator,
        const std::array<std::int16_t, inputHidden> &bias,
        const std::uint32_t firstDelta,
        const std::uint32_t secondDelta)
    {
        // Subtract the bias from the input arrays:
        for (std::uint16_t i = 0; i < hiddenSize; i++)
        {
            firstAccumulator[i] -= bias[firstDelta + i];
        }
        for (std::uint16_t i = 0; i < hiddenSize; i++)
        {
            secondAccumulator[i] -= bias[secondDelta + i];
        }
    }

    static inline void subAddAll(
        std::array<std::int16_t, hiddenSize> &firstAccumulator,
        std::array<std::int16_t, hiddenSize> &secondAccumulator,
        const std::array<std::int16_t, inputHidden> &bias,
        const std::uint32_t firstBiasWithSub,
        const std::uint32_t firstBiasWithAdd,
        const std::uint32_t secondBiasWithSub,
        const std::uint32_t secondBiasWithAdd)
    {
        // Subtract and add the bias to the input arrays:
        for (std::uint16_t i = 0; i < hiddenSize; i++)
        {
            firstAccumulator[i] = firstAccumulator[i] - bias[firstBiasWithSub + i] + bias[firstBiasWithAdd + i];
            secondAccumulator[i] = secondAccumulator[i] - bias[secondBiasWithSub + i] + bias[secondBiasWithAdd + i];
        }
    }

    static void activate(
        const std::array<std::int16_t, hiddenSize> &firstAccumulator,
        const std::array<std::int16_t, hiddenSize> &secondAccumulator,
        const std::array<std::int16_t, hiddenSize * 2 * outputSize> &weight,
        const std::array<std::int16_t, outputSize> &bias,
        std::array<std::int32_t, outputSize> &output)
    {
        std::uint16_t step = 0;
        for (std::uint16_t i = 0; i < outputSize; i++)
        {
            std::int32_t sum = 0;

            for (std::uint16_t j = 0; j < hiddenSize; j++)
            {
                // Activate everything
                sum += crelu(firstAccumulator[j]) * weight[step + j];
                sum += crelu(secondAccumulator[j]) * weight[hiddenSize + step + j];
            }
            step += hiddenSize * 2;
            sum /= QA;
            output[i] = sum + bias[i];
        }
    }
};