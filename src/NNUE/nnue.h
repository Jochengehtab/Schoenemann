/*
  This file is part of the Schoenemann chess engine written by Jochengehtab

  Copyright (C) 2024-2025 Jochengehtab

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU Affero General Public License as
  published by the Free Software Foundation, either version 3 of the
  License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Affero General Public License for more details.

  You should have received a copy of the GNU Affero General Public License
  along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef NNUE_H
#define NNUE_H

#include <array>
#include <cstdint>
#include <cassert>
#include <sstream>
#include <immintrin.h>

#include "accumulator.h"
#include "utils.h"
#include "incbin.h"

class Network
{
private:
    struct
    {
        std::array<std::int16_t, inputHiddenSize> featureWeight;
        std::array<std::int16_t, hiddenSize> featureBias;

        std::array<std::array<std::int16_t, hiddenSize * 2>, outputSize> outputWeight;
        std::array<std::int16_t, outputSize> outputBias;
    } innerNet;

    accumulator acc;

public:
    Network()
    {

        // Open the NNUE file with the given path
        FILE *nn;
        #if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
            fopen_s(&nn, EVALFILE, "rb");
        #else
            nn = fopen(EVALFILE, "rb");
        #endif

        if (nn)
        {
            size_t read = 0;
            size_t fileSize = sizeof(innerNet);
            size_t objectsExpected = fileSize / sizeof(int16_t);

            // Read all the different weight and bias
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

    inline void refreshAccumulator()
    {
        acc.zeroAccumulator();
        acc.loadBias(innerNet.featureBias);
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

        // Update the accumolator
        if (operation == activate)
        {
            util::addAll(acc.white, acc.black, innerNet.featureWeight, whiteIndex * hiddenSize, blackIndex * hiddenSize);
        }
        else
        {
            util::subAll(acc.white, acc.black, innerNet.featureWeight, whiteIndex * hiddenSize, blackIndex * hiddenSize);
        }
    }

    inline std::int32_t evaluate(const std::uint8_t sideToMove, int pieces)
    {

        // Calculate the bucket based on the number of pieces on the board
        const short bucket = (pieces - 2) / ((32 + outputSize - 1) / outputSize);

        int eval = 0;

        // Perform a forward pass throw the network
        if (sideToMove == 0)
        {
            eval = util::forward(acc.white, acc.black, innerNet.outputWeight, innerNet.outputBias, bucket);
        }
        else
        {
            eval = util::forward(acc.black, acc.white, innerNet.outputWeight, innerNet.outputBias, bucket);
        }
        
        return eval;
    }
};

#endif