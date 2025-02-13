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

#pragma once

#include <array>
#include <immintrin.h>

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
#ifdef __AVX2__
        const __m256i vecZero = _mm256_setzero_si256();
        const __m256i vecQA = _mm256_set1_epi16(QA);
        __m256i sum = vecZero;

        for (int i = 0; i < hiddenSize; i += 16)
        {
            const __m256i usVec = _mm256_loadu_si256((const __m256i *)(&us[i]));
            const __m256i themVec = _mm256_loadu_si256((const __m256i *)(&them[i]));
            const __m256i usWeights = _mm256_loadu_si256((const __m256i *)(&outputWeight[bucket][i]));
            const __m256i themWeights = _mm256_loadu_si256((const __m256i *)(&outputWeight[bucket][i + hiddenSize]));

            // Clamp all the values using _mm256_min_epi16
            const __m256i usClamped = _mm256_min_epi16(_mm256_max_epi16(usVec, vecZero), vecQA);
            const __m256i themClamped = _mm256_min_epi16(_mm256_max_epi16(themVec, vecZero), vecQA);

            const __m256i usResults = _mm256_madd_epi16(_mm256_mullo_epi16(usWeights, usClamped), usClamped);
            const __m256i themResults = _mm256_madd_epi16(_mm256_mullo_epi16(themWeights, themClamped), themClamped);

            sum = _mm256_add_epi32(sum, usResults);
            sum = _mm256_add_epi32(sum, themResults);
        }

        __m256i vecOne = _mm256_hadd_epi32(sum, sum);
        __m256i vecTwo = _mm256_hadd_epi32(vecOne, vecOne);

        eval = _mm256_extract_epi32(vecTwo, 0) + _mm256_extract_epi32(vecTwo, 4);
#else
        for (std::uint16_t i = 0; i < hiddenSize; i++)
        {
            eval += screlu(us[i]) * outputWeight[bucket][i] +
                    screlu(them[i]) * outputWeight[bucket][i + hiddenSize];
        }
#endif
        eval /= QA;
        eval += outputBias[bucket];
        eval *= scale;
        eval /= (QA * QB);
        return eval;
    }
};