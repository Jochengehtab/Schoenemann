#pragma once

#include <array>
#include <cstdint>
#include <cassert>
#include <sstream>

#include "PerspectiveAccumulator.h"
#include "utils.h"
#include "BinaryFileStream.h"

enum class operations
{
    Activate,
    Deactivate
};

class crelu
{
    public:
        static inline std::int16_t Activate(int input)
        {
            return std::max(0, std::min(255, input));
        }
};

class network
{
private:
    std::array<std::int16_t, inputSize * hiddenSize> FeatureWeight;
    std::array<std::int16_t, hiddenSize> FeatureBias;
    std::array<std::int16_t, hiddenSize * 2 * outputSize> OutputWeight;
    std::array<std::int16_t, outputSize> OutputBias;
    std::array<std::int32_t, outputSize> Output;

    std::array<PerspectiveAccumulator, stackSize> accumulators;
    uint16_t currentAccumulator = 0;

    
    void InitializeAccumulatorStack()
    {
        PerspectiveAccumulator accumulator;
        std::fill(std::begin(accumulators), std::end(accumulators), accumulator);
    }

public:
    
    network()
    {
        InitializeAccumulatorStack();
    }

    // Read the in memory network which is sotred in a headerfile
    explicit network(BinaryFileStream &stream)
    {
        InitializeAccumulatorStack();

        stream.ReadArray(FeatureWeight);
        stream.ReadArray(FeatureBias);
        stream.ReadArray(OutputWeight);
        stream.ReadArray(OutputBias);
    }
    
    inline void ResetAccumulator()
    {
        currentAccumulator = 0;
    }

    inline void RefreshAccumulator()
    {
        PerspectiveAccumulator &accumulator = accumulators[currentAccumulator];
        accumulator.zeroAccumulator();
        accumulator.loadBias(FeatureBias);
    }

    inline void efficientlyUpdateAccumulator(const uint8_t piece, const uint8_t color, const uint8_t from, const uint8_t to)
    {
        // Calculate the stride necessary to get to the correct piece:
        const uint16_t pieceStride = piece * PieceStride;

        // Calculate the indices for the square of the piece that is moved with respect to both perspectives:
        const uint32_t whiteIndexFrom = color * ColorStride + pieceStride + from;
        const uint32_t blackIndexFrom = (color ^ 1) * ColorStride + pieceStride + (from ^ 56);

        // Calculate the indices for the square the piece is moved to with respect to both perspectives:
        const uint32_t whiteIndexTo = color * ColorStride + pieceStride + to;
        const uint32_t blackIndexTo = (color ^ 1) * ColorStride + pieceStride + (to ^ 56);

        // Fetch the current accumulator:
        PerspectiveAccumulator &accumulator = accumulators[currentAccumulator];

        // Efficiently update the accumulator:
        utilitys::subAddAll(accumulator.white, accumulator.black,
                                  FeatureWeight,
                                  whiteIndexFrom * hiddenSize,
                                  whiteIndexTo * hiddenSize,
                                  blackIndexFrom * hiddenSize,
                                  blackIndexTo * hiddenSize);
    }

    template <operations Operation>
    inline void efficientlyUpdateAccumulator(const uint8_t piece, const uint8_t color,
                                             const uint8_t sq)
    {
        // Calculate the stride necessary to get to the correct piece:
        const uint16_t pieceStride = piece * PieceStride;

        // Calculate the indices for the square of the piece that is inserted or removed with respect to both
        // perspectives:
        const uint32_t whiteIndex = color * ColorStride + pieceStride + sq;
        const uint32_t blackIndex = (color ^ 1) * ColorStride + pieceStride + (sq ^ 56);

        // Fetch the current accumulator:
        PerspectiveAccumulator &accumulator = accumulators[currentAccumulator];

        // Update the accumolator
        if (Operation == operations::Activate)
        {
            utilitys::addAll(accumulator.white, accumulator.black, FeatureWeight, whiteIndex * hiddenSize, blackIndex * hiddenSize);
        }
        else
        {
            utilitys::subAll(accumulator.white, accumulator.black, FeatureWeight, whiteIndex * hiddenSize, blackIndex * hiddenSize);
        }
    }
    
    inline std::int32_t evaluate(const uint8_t colorToMove)
    {
        // Fetch the current accumulator:
        PerspectiveAccumulator &accumulator = accumulators[currentAccumulator];

        // Activate, flatten, and forward-propagate the accumulator to evaluate the network:
        if (colorToMove == 0)
            utilitys::activate<crelu>(accumulator.white, accumulator.black, OutputWeight, OutputBias, Output, 0);
        else
            utilitys::activate<crelu>(accumulator.black, accumulator.white, OutputWeight, OutputBias, Output, 0);

        // Scale the output with respect to the quantization and return it:
        return Output[0] * scale / (QA * QB);
    }
};
