#pragma once

#include <array>
#include <cstdint>
#include <cassert>
#include <sstream>

#include "accumulator.h"
#include "utils.h"
#include "stream.h"

class network
{
private:
    std::array<std::int16_t, inputHidden> featureWeight;
    std::array<std::int16_t, hiddenSize> featureBias;

    std::array<std::int16_t, hiddenSize * 2 * outputSize> outputWeight;
    std::array<std::int16_t, outputSize> outputBias;

    std::array<std::int32_t, outputSize> finalOutput;

    std::array<accumulator, 1> accumulators;
    std::uint16_t currentAccumulator = 0;

    void initAccumulator()
    {
        accumulator accumulator;
        std::fill(std::begin(accumulators), std::end(accumulators), accumulator);
    }

public:
    network()
    {
        initAccumulator();
    }

    // Read the in memory network which is stored in a headerfile
    explicit network(memorystream &stream)
    {
        initAccumulator();

        stream.readArray(featureWeight);
        stream.readArray(featureBias);
        stream.readArray(outputWeight);
        stream.readArray(outputBias);
    }

    inline void resetAccumulator()
    {
        currentAccumulator = 0;
    }

    inline void refreshAccumulator()
    {
        accumulator &accumulator = accumulators[currentAccumulator];
        accumulator.zeroAccumulator();
        accumulator.loadBias(featureBias);
    }

    inline void updateAccumulator(
        const std::uint8_t piece,
        const std::uint8_t color,
        const std::uint8_t sq,
        const bool operation)
    {
        // Calculate the stride necessary to get to the correct piece:
        const std::uint16_t pieceIndex = piece * whiteSquares;

        // Get the squre index based on the color
        const std::uint16_t whiteIndex = color * blackSqures + pieceIndex + sq;
        const std::uint16_t blackIndex = (color ^ 1) * blackSqures + pieceIndex + (sq ^ 56);

        // Get the accumulatror
        accumulator &accumulator = accumulators[currentAccumulator];

        // Update the accumolator
        if (operation == activate)
        {
            utilitys::addAll(accumulator.white, accumulator.black, featureWeight, whiteIndex * hiddenSize, blackIndex * hiddenSize);
        }
        else
        {
            utilitys::subAll(accumulator.white, accumulator.black, featureWeight, whiteIndex * hiddenSize, blackIndex * hiddenSize);
        }
    }

    inline std::int32_t evaluate(const std::uint8_t sideToMove)
    {
        // Get the accumulatror
        accumulator &accumulator = accumulators[currentAccumulator];

        // Make a forward pass throw the network based on the sideToMove
        if (sideToMove == 0)
        {
            utilitys::activate(accumulator.white, accumulator.black, outputWeight, outputBias, finalOutput);
        }
        else
        {
            utilitys::activate(accumulator.black, accumulator.white, outputWeight, outputBias, finalOutput);
        }

        // Scale ouput and dived it by QAB
        return finalOutput[0] * scale / (QA * QB);
    }
};
