#pragma once

#include <array>
#include <cstdint>
#include <cassert>
#include <sstream>
#include <immintrin.h>

#include "accumulator.h"
#include "utils.h"
#include "incbin.h"

class network
{
private:
    struct
    {
        std::array<std::int16_t, inputHiddenSize> featureWeight;
        std::array<std::int16_t, hiddenSize> featureBias;

        std::array<std::array<std::int16_t, hiddenSize * 2>, outputSize> outputWeight;
        std::array<std::int16_t, outputSize> outputBias;
    } innerNet;

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

        // Open the NNUE file with the given path
        FILE *nn = fopen(EVALFILE, "rb");

        if (nn)
        {
            size_t read = 0;
            size_t fileSize = sizeof(innerNet);
            size_t objectsExpected = fileSize / sizeof(int16_t);

            read += fread(&innerNet.featureWeight, sizeof(int16_t), inputSize * hiddenSize, nn);
            read += fread(&innerNet.featureBias, sizeof(int16_t), hiddenSize, nn);
            read += fread(&innerNet.outputWeight, sizeof(int16_t), hiddenSize * 2 * outputSize, nn);
            read += fread(&innerNet.outputBias, sizeof(int16_t), outputSize, nn);

            // Check if the file was read correctly
            if (std::abs((int64_t)read - (int64_t)objectsExpected) >= 16)
            {
                std::cout << "Error loading the net, aborting ";
                std::cout << "Expected " << objectsExpected << " shorts, got " << read << "\n";
                exit(1);
            }

            // Close the file after reading it
            fclose(nn);
        }
        else
        {
            std::cout << "The NNUE File wasn't found" << std::endl;
            exit(1);
        }
    }

    inline void resetAccumulator()
    {
        currentAccumulator = 0;
    }

    inline void refreshAccumulator()
    {
        accumulator &accumulator = accumulators[currentAccumulator];
        accumulator.zeroAccumulator();
        accumulator.loadBias(innerNet.featureBias);
    }

    inline void updateAccumulator(
        const std::uint8_t piece,
        const std::uint8_t color,
        const std::uint8_t square,
        const bool operation)
    {
        // Calculate the stride necessary to get to the correct piece:
        const std::uint16_t pieceIndex = piece * whiteSquares;

        // Get the squre index based on the color
        const std::uint16_t whiteIndex = color * blackSqures + pieceIndex + square;
        const std::uint16_t blackIndex = (color ^ 1) * blackSqures + pieceIndex + (square ^ 56);

        // Get the accumulatror
        accumulator &accumulator = accumulators[currentAccumulator];

        // Update the accumolator
        if (operation == activate)
        {
            util::addAll(accumulator.white, accumulator.black, innerNet.featureWeight, whiteIndex * hiddenSize, blackIndex * hiddenSize);
        }
        else
        {
            util::subAll(accumulator.white, accumulator.black, innerNet.featureWeight, whiteIndex * hiddenSize, blackIndex * hiddenSize);
        }
    }

    inline std::int32_t evaluate(const std::uint8_t sideToMove, int pieces)
    {
        // Get the accumulatror
        accumulator &accumulator = accumulators[currentAccumulator];

        const short bucket = (pieces - 2) / ((32 + outputSize - 1) / outputSize);

        int eval = 0;

        // If the side to move is white we evaluate for white
        if (sideToMove == 0)
        {
            eval = util::forward(accumulator.white, accumulator.black, innerNet.outputWeight, innerNet.outputBias, bucket);
        }
        else
        {
            eval = util::forward(accumulator.black, accumulator.white, innerNet.outputWeight, innerNet.outputBias, bucket);
        }
        return eval;
    }
};
