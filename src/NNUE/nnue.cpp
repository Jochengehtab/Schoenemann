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


#include "nnue.h"

#include <cstring>
#include <cassert>

INCBIN(NETWORK, EVALFILE);

Network::Network()
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
        // Use the embedded network
        std::memcpy(&innerNet, gNETWORKData, sizeof(innerNet));
    }
}

