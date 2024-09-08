#pragma once

#include <array>

class utilitys
{
public:
    
    template <typename T, size_t InputSize, size_t DeltaSize>
    static inline void addAll(std::array<T, InputSize> &inputA, std::array<T, InputSize> &inputB,
                                const std::array<T, DeltaSize> &delta,
                                const uint32_t oA, const uint32_t oB)
    {
        // Add the delta to the input arrays:
        for (size_t i = 0; i < InputSize; i++)
        {
            inputA[i] += delta[oA + i];
        }
        for (size_t i = 0; i < InputSize; i++)
        {
            inputB[i] += delta[oB + i];
        }
    }

    
    template <typename T, size_t InputSize, size_t DeltaSize>
    static inline void subAll(std::array<T, InputSize> &inputA, std::array<T, InputSize> &inputB,
                                       const std::array<T, DeltaSize> &delta,
                                       const uint32_t oA, const uint32_t oB)
    {
        // Subtract the delta from the input arrays:
        for (size_t i = 0; i < InputSize; i++)
        {
            inputA[i] -= delta[oA + i];
        }
        for (size_t i = 0; i < InputSize; i++)
        {
            inputB[i] -= delta[oB + i];
        }
    }

    template <typename T, size_t InputSize, size_t DeltaSize>
    static inline void subAddAll(std::array<T, InputSize> &inputA, std::array<T, InputSize> &inputB,
                                           const std::array<T, DeltaSize> &delta,
                                           const uint32_t oAS, const uint32_t oAA,
                                           const uint32_t oBS, const uint32_t oBA)
    {
        // Subtract and add the delta to the input arrays:
        for (size_t i = 0; i < InputSize; i++)
        {
            inputA[i] = inputA[i] - delta[oAS + i] + delta[oAA + i];
            inputB[i] = inputB[i] - delta[oBS + i] + delta[oBA + i];
        }
    }

    template <typename Activation, typename T, typename OT, size_t InputSize, size_t OutputSize>
    static void activate(
        const std::array<T, InputSize> &inputA, const std::array<T, InputSize> &inputB,
        const std::array<T, InputSize * 2 * OutputSize> &weight,
        const std::array<T, OutputSize> &bias,
        std::array<OT, OutputSize> &output, const uint32_t o)
    {
        // Define the stride with respect to the weight array:
        size_t stride = 0;

        // Perform the joint activation-flattening-forward propagation using matrix multiplication, defined as
        // output = activation(flatten(input)) * weight + bias:
        for (size_t i = 0; i < OutputSize; i++)
        {
            // Define the sum accumulation variable:
            OT sum = 0;

            for (size_t j = 0; j < InputSize; j++)
            {
                // Add the activation of the input multiplied by the weight to the sum:
                sum += Activation::Activate(inputA[j]) * weight[stride + j];
                sum += Activation::Activate(inputB[j]) * weight[InputSize + stride + j];
            }

            // Stride to the next set of weights:
            stride += InputSize * 2;

            // Store the sum with respect to the bias:
            output[o + i] = sum + bias[o + i];
        }
    }
};