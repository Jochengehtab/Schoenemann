//
// Copyright (c) 2023 MantaRay authors. See the list of authors for more details.
// Licensed under MIT.
//

#ifndef MANTARAY_PERSPECTIVEACCUMULATOR_H
#define MANTARAY_PERSPECTIVEACCUMULATOR_H

#include <array>

namespace MantaRay
{

    /// \brief The Perspective-accounting Accumulator.
    /// \tparam T The internal type of the accumulator.
    /// \tparam AccumulatorSize The size of the accumulator.
    /// \details The accumulator is used to store the input layer activations of the Perspective-accounting
    ///          Neural Network and allow efficient updates to the activations (other activations / deactivations).
    ///          The accumulator is internally aligned to architecture-specific boundaries to allow for efficient
    ///          SIMD operations.
    /// \see MantaRay::AccumulatorOperation for the operations that can be performed on the accumulator.
    /// \see MantaRay::PerspectiveNNUE for the Neural Network implementation that uses this accumulator.
    template<typename T, size_t AccumulatorSize>
    class PerspectiveAccumulator
    {

        public:
            std::array<T, AccumulatorSize> White;
            std::array<T, AccumulatorSize> Black;

            /// \brief Default constructor.
            /// \details Initializes the accumulator to zero.
            PerspectiveAccumulator()
            {
                Zero();
            }

            /// \brief Copy method for the accumulator.
            /// \param accumulator The accumulator to copy to.
            /// \details Copies the contents of this accumulator to the provided accumulator. Uses SIMD instructions
            ///          where beneficial.
            inline void CopyTo(PerspectiveAccumulator<T, AccumulatorSize>& accumulator)
            {
                std::copy(std::begin(White), std::end(White), std::begin(accumulator.White));
                std::copy(std::begin(Black), std::end(Black), std::begin(accumulator.Black));
            }

            /// \brief Loads the bias into the accumulator.
            /// \param bias The bias to load.
            /// \details This method is used to load the bias into the accumulator. This is once at the start to
            ///          properly initialize the accumulator, and prevents having to load the bias every time the
            ///          accumulator is updated or inferred from.
            inline void LoadBias(std::array<T, AccumulatorSize>& bias)
            {
                std::copy(std::begin(bias), std::end(bias), std::begin(White));
                std::copy(std::begin(bias), std::end(bias), std::begin(Black));
            }

            /// \brief Zeroes out the accumulator.
            /// \details This method is used to zero out the accumulator. This is done once at the start to avoid any
            ///          potential issues with uninitialized or unchecked memory.
            inline void Zero()
            {
                std::fill(std::begin(White), std::end(White), 0);
                std::fill(std::begin(Black), std::end(Black), 0);
            }

    };

} // MantaRay

#endif //MANTARAY_PERSPECTIVEACCUMULATOR_H
