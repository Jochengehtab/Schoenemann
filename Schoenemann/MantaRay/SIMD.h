//
// Copyright (c) 2023 MantaRay authors. See the list of authors for more details.
// Licensed under MIT.
//

#ifndef MANTARAY_SIMD_H
#define MANTARAY_SIMD_H

#include <array>

namespace MantaRay
{

    /// \brief SIMD implementations for MantaRay.
    /// \details This class contains SIMD implementations of common operations used in MantaRay.
    class SIMD
    {

        public:
            /// \brief Add the delta to elements in the input arrays.
            /// \tparam T The type of the input and delta.
            /// \tparam InputSize The size of the input arrays.
            /// \tparam DeltaSize The size of the delta array.
            /// \param inputA The first input array.
            /// \param inputB The second input array.
            /// \param delta The delta array.
            /// \param oA The delta offset for the first input array.
            /// \param oB The delta offset for the second input array.
            /// \details This function adds the offset delta to elements in the input arrays.
            ///          The delta is added to the input arrays in-place.
            template<typename T, size_t InputSize, size_t DeltaSize>
            static inline void AddToAll(std::array<T, InputSize>& inputA, std::array<T, InputSize>& inputB,
                                        const std::array<T, DeltaSize>& delta,
                                        const uint32_t oA, const uint32_t oB)
            {
                // Add the delta to the input arrays:
                for (size_t i = 0; i < InputSize; i++) inputA[i] += delta[oA + i];
                for (size_t i = 0; i < InputSize; i++) inputB[i] += delta[oB + i];
            }

            /// \brief Subtract the delta from elements in the input arrays.
            /// \tparam T The type of the input and delta.
            /// \tparam InputSize The size of the input arrays.
            /// \tparam DeltaSize The size of the delta array.
            /// \param inputA The first input array.
            /// \param inputB The second input array.
            /// \param delta The delta array.
            /// \param oA The delta offset for the first input array.
            /// \param oB The delta offset for the second input array.
            /// \details This function subtracts the offset delta from elements in the input arrays.
            ///          The delta is subtracted from the input arrays in-place.
            template<typename T, size_t InputSize, size_t DeltaSize>
            static inline void SubtractFromAll(std::array<T, InputSize>& inputA, std::array<T, InputSize>& inputB,
                                               const std::array<T, DeltaSize>& delta,
                                               const uint32_t oA, const uint32_t oB)
            {

                for (size_t i = 0; i < InputSize; i++) inputA[i] -= delta[oA + i];
                for (size_t i = 0; i < InputSize; i++) inputB[i] -= delta[oB + i];
            }

            /// \brief Combination of SubtractFromAll and AddToAll.
            /// \tparam T The type of the input and delta.
            /// \tparam InputSize The size of the input arrays.
            /// \tparam DeltaSize The size of the delta array.
            /// \param inputA The first input array.
            /// \param inputB The second input array.
            /// \param delta The delta array.
            /// \param oAS The delta offset for the first input array with respect to subtraction.
            /// \param oAA The delta offset for the first input array with respect to addition.
            /// \param oBS The delta offset for the second input array with respect to subtraction.
            /// \param oBA The delta offset for the second input array with respect to addition.
            /// \details This function subtracts the offset delta from elements in the input arrays.
            ///          The delta is subtracted from the input arrays in-place.
            ///          Then another offset delta is added to the input arrays.
            ///          The delta is added to the input arrays in-place.
            template<typename T, size_t InputSize, size_t DeltaSize>
            static inline void SubtractAndAddToAll(std::array<T, InputSize>& inputA, std::array<T, InputSize>& inputB,
                                                   const std::array<T, DeltaSize>& delta,
                                                   const uint32_t oAS, const uint32_t oAA,
                                                   const uint32_t oBS, const uint32_t oBA)
            {
                // Subtract and add the delta to the input arrays:
                for (size_t i = 0; i < InputSize; i++) {
                    inputA[i] = inputA[i] - delta[oAS + i] + delta[oAA + i];
                    inputB[i] = inputB[i] - delta[oBS + i] + delta[oBA + i];
                }
            }

            /// \brief Activate the input arrays, flatten the concatenated tensor result, and forward propagate the
            ///        flattened result.
            /// \tparam Activation The activation function to use.
            /// \tparam T The type of the input, weight, and bias arrays.
            /// \tparam OT The type of the output array.
            /// \tparam InputSize The size of the input arrays.
            /// \tparam OutputSize The size of the output array.
            /// \param inputA The first input array.
            /// \param inputB The second input array.
            /// \param weight The weight array.
            /// \param bias The bias array.
            /// \param output The output array.
            /// \param o The offset into the output array.
            /// \details This function activates the input arrays. Then it creates a Tensor-view of the input arrays,
            ///          concatenating them vertically. After which, it flattens the vertical tensor into a 1D tensor.
            ///          Finally, it forwards propagates the flattened tensor with respect to the weight and bias arrays
            ///          using simple matrix multiplication. The result is stored in the output array starting at the
            ///          given offset.
            template<typename Activation, typename T, typename OT, size_t InputSize, size_t OutputSize>
            [[clang::noinline]]
            static void ActivateFlattenAndForward(
                    const std::array<T, InputSize>& inputA, const std::array<T, InputSize>& inputB,
                    const std::array<T, InputSize * 2 * OutputSize>& weight,
                    const std::array<T, OutputSize>& bias,
                    std::array<OT, OutputSize>& output, const uint32_t o)
            {
                // Define the stride with respect to the weight array:
                size_t stride = 0;

                // Perform the joint activation-flattening-forward propagation using matrix multiplication, defined as
                // output = activation(flatten(input)) * weight + bias:
                for (size_t i = 0; i < OutputSize; i++) {
                    // Define the sum accumulation variable:
                    OT sum = 0;

                    for (size_t j = 0; j < InputSize; j++) {
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
} // MantaRay

#endif //MANTARAY_SIMD_H
