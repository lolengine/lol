//
//  Lol Engine
//
//  Copyright © 2010–2024 Sam Hocevar <sam@hocevar.net>
//              2024 NuSan
//
//  Lol Engine is free software. It comes without any warranty, to
//  the extent permitted by applicable law. You can redistribute it
//  and/or modify it under the terms of the Do What the Fuck You Want
//  to Public License, Version 2, as published by the WTFPL Task Force.
//  See http://www.wtfpl.net/ for more details.
//

#pragma once

//
// Interpolation classes
// —————————————————————
//

#include <cmath> // std::sin
#include "../math/constants.h" // F_PI

namespace lol::interp
{

// Lanczos interpolation
// SIZE: number of lobes of the sinc function that contribute to the interpolation
// PRECISION: number of samples per lobe in the kernel
template<typename T, size_t SIZE = 16, size_t PRECISION = 64>
class lanczos
{
public:
    lanczos()
    {
        for (size_t k = 0; k < SIZE * PRECISION; ++k)
        {
            T dist = T(k) * F_PI / m_scale;
            m_kernel[k] = dist ? m_center * std::sin(dist) * std::sin(dist / m_center) / (dist * dist) : T(1);
        }
    }

    T get(T *data, size_t stride, T offset) const
    {
        T ret(0);
        for (size_t k = 0; k < SIZE; ++k)
        {
            float dist = std::abs(k - m_center - offset);
            ret += data[k * stride] * m_kernel[size_t(dist * m_scale)];
        }
        return ret;
    }

    size_t const size() const { return SIZE; }

private:
    static inline T const m_center = SIZE / 2;
    static inline T const m_scale = (SIZE * PRECISION - 1) / (m_center + 1);

    std::array<T, SIZE * PRECISION> m_kernel;
};

} // namespace lol::interp
