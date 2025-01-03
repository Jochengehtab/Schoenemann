#pragma once

#include <array>
#include <cstdint>
#include <cassert>
#include <sstream>
#include <immintrin.h>

#include "accumulator.h"
#include "utils.h"
#include "stream.h"
class network
{
private:
    struct
    {
        std::array<std::int16_t, inputHidden> featureWeight;
        std::array<std::int16_t, hiddenSize> featureBias;

        std::array<std::array<std::int16_t, outputSize>, hiddenSize * 2> outputWeight;
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
    }

    // Read the in memory network which is sotred in a headerfile
    explicit network(memorystream &stream)
    {
        initAccumulator();

        // open the nn file
        FILE *nn;
        fopen_s(&nn, "C:\\GitHub\\Schoenemann\\src\\quantised.bin", "rb");
        ;
        // FILE *nn = nullptr;

        // if it's not invalid read the config values from it
        if (nn)
        {
            std::cout << "Reading network file" << std::endl;
            // initialize an accumulator for every input of the second layer
            size_t read = 0;
            size_t fileSize = sizeof(innerNet);
            size_t objectsExpected = fileSize / sizeof(int16_t);

            read += fread(&innerNet.featureWeight, sizeof(int16_t), inputSize * hiddenSize, nn);
            read += fread(&innerNet.featureBias, sizeof(int16_t), hiddenSize, nn);
            std::array<std::array<std::int16_t, hiddenSize * 2>, outputSize> transposed;
            read += fread(&transposed, sizeof(int16_t), hiddenSize * 2 * outputSize, nn);
            read += fread(&innerNet.outputBias, sizeof(int16_t), outputSize, nn);


            for (std::size_t i = 0; i < outputSize;i++)
            {
                for (std::size_t j = 0; j < hiddenSize * 2; j++)
                {
                    innerNet.outputWeight[i][j] = transposed[i][j];
                }
            }

            if (std::abs((int64_t)read - (int64_t)objectsExpected) >= 16)
            {
                std::cout << "Error loading the net, aborting ";
                std::cout << "Expected " << objectsExpected << " shorts, got " << read << "\n";
                exit(1);
            }

            // after reading the config we can close the file
            fclose(nn);
        }
        else
        {
            // std::cout << "Using the default loading method" << std::endl;
            stream.readArray(innerNet.featureWeight);
            stream.readArray(innerNet.featureBias);
            // stream.readArray(innerNet.outputWeight);
            stream.readArray(innerNet.outputBias);
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
            utilitys::addAll(accumulator.white, accumulator.black, innerNet.featureWeight, whiteIndex * hiddenSize, blackIndex * hiddenSize);
        }
        else
        {
            utilitys::subAll(accumulator.white, accumulator.black, innerNet.featureWeight, whiteIndex * hiddenSize, blackIndex * hiddenSize);
        }
    }

    inline std::int32_t evaluate(const std::uint8_t sideToMove, int whitePieces, int blackPieces)
    {
        // Get the accumulatror
        accumulator &accumulator = accumulators[currentAccumulator];

        int bucket = 0;
        if (sideToMove == 0)
        {
            bucket = (whitePieces - 2) / ((32 + outputSize - 1) / outputSize);
        }
        else
        {
            bucket = (blackPieces - 2) / ((32 + outputSize - 1) / outputSize);
        }

        // Make a forward pass throw the network based on the sideToMove
        int eval = 0;
        if (sideToMove == 0)
        {
            eval = utilitys::activate(accumulator.white, accumulator.black, innerNet.outputWeight, innerNet.outputBias, bucket);
        }
        else
        {
            eval = utilitys::activate(accumulator.black, accumulator.white, innerNet.outputWeight, innerNet.outputBias, bucket);
        }

        // std::cout << std::endl;

        // std::cout << "White amount pieces: " << whitePieces << std::endl;
        // std::cout << "Black amount pieces: " << blackPieces << std::endl;

        // std::cout << "The bucket is: " << bucket << std::endl;

        // Scale ouput and dived it by QAB
        return eval;
    }
};
