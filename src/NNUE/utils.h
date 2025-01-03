#pragma once

#include <array>

#include "nnueconstants.h"

class utilitys
{
public:
    static inline std::int32_t screlu(int input)
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

    static int activate(
        const std::array<std::int16_t, hiddenSize> &firstAccumulator,
        const std::array<std::int16_t, hiddenSize> &secondAccumulator,
        const std::array<std::array<std::int16_t, outputSize>, hiddenSize * 2> outputWeight,
        const std::array<std::int16_t, outputSize> &bias,
        std::array<std::int32_t, outputSize> &output,
        int bucket)
    {
        int eval = 0;
        for (std::uint16_t i = 0; i < outputSize; i++)
        {
            int step = 0;
            for (std::uint16_t j = 0; j < hiddenSize; j++)
            {
                int ourEval = screlu(firstAccumulator[j] * outputWeight[bucket][j + step]);
                int theirEval = screlu(secondAccumulator[j] * outputWeight[bucket][j + step]);
                eval += ourEval + theirEval;
                step += hiddenSize * 2;
            eval /= QA;
            eval += bias[bucket];
        eval *= scale;
        eval /= (QA * QB);
            }
        

        }
        return eval;
    }
};