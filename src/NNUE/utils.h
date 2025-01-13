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
        const __m256i vec_zero = _mm256_setzero_si256();
        const __m256i vec_qa = _mm256_set1_epi16(QA);
        __m256i sum = vec_zero;

        for (int i = 0; i < hiddenSize; i++)
        {
            const __m256i us_vec = _mm256_loadu_si256((const __m256i *)(us.data() + i));
        const __m256i them_vec = _mm256_loadu_si256((const __m256i *)(them.data() + i));
        const __m256i us_weights = _mm256_loadu_si256((const __m256i *)(outputWeight[bucket].data() + i));
        const __m256i them_weights = _mm256_loadu_si256((const __m256i *)(outputWeight[bucket].data() + i + hiddenSize));

        const __m256i us_clamped = _mm256_min_epi16(_mm256_max_epi16(us_vec, vec_zero), vec_qa);
        const __m256i them_clamped = _mm256_min_epi16(_mm256_max_epi16(them_vec, vec_zero), vec_qa);

        const __m256i us_results = _mm256_madd_epi16(_mm256_mullo_epi16(us_weights, us_clamped), us_clamped);
        const __m256i them_results = _mm256_madd_epi16(_mm256_mullo_epi16(them_weights, them_clamped), them_clamped);

        sum = _mm256_add_epi32(sum, us_results);
        sum = _mm256_add_epi32(sum, them_results);
    }
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