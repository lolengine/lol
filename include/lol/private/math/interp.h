//
//  Lol Engine
//
//  Copyright © 2010–2024 Sam Hocevar <sam@hocevar.net>
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

template<typename T, size_t SIZE = 16, size_t PRECISION = 64>
class lanczos
{
public:
    lanczos()
    {
        T const window_center = SIZE / 2;
        T const lut_scale = (SIZE * PRECISION - 1) / (window_center + 1);
        for (size_t k = 0; k < SIZE * PRECISION; ++k)
        {
            T dist = T(k) * F_PI / lut_scale;
            m_lut[k] = dist ? window_center * std::sin(dist) * std::sin(dist / window_center) / (dist * dist) : T(1);
        }
    }

    T get(T *data, size_t stride, T alpha)
    {
        T const window_center = SIZE / 2;
        T const lut_scale = (SIZE * PRECISION - 1) / (window_center + 1);
        T ret(0);
        for (size_t k = 0; k < SIZE; ++k)
        {
            float dist = std::abs(k - window_center - alpha);
            float lanczos = m_lut[size_t(dist * lut_scale)];
            ret += data[k * stride] * lanczos;
        }
        return ret;
    }

    size_t const size() { return SIZE; }

private:
    static size_t const center = T(SIZE) / 2;

    std::array<T, SIZE * PRECISION> m_lut;
};

} // namespace lol::interp
