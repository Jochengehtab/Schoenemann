//
// Copyright (c) 2023 MantaRay authors. See the list of authors for more details.
// Licensed under MIT.
//

#ifndef MANTARAY_CLIPPEDRELU_H
#define MANTARAY_CLIPPEDRELU_H

#include <utility>

namespace MantaRay
{

    /// \brief Clipped Rectified Linear Unit (ReLU) activation function.
    /// \tparam T The type of the input and output.
    /// \tparam Minimum The minimum value to return.
    /// \tparam Maximum The maximum value to return.
    /// \details Calling the static Activate function will provide the clipped ReLU of the input.
    ///          The input is clipped to the range [Minimum, Maximum].
    template<typename T, T Minimum, T Maximum>
    class ClippedReLU
    {

        public:
            static inline T Activate(const T arg)
            {
                return std::max(Minimum, std::min(Maximum, arg));
            }

    };

} // MantaRay

#endif //MANTARAY_CLIPPEDRELU_H
