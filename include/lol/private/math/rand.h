//
//  Lol Engine
//
//  Copyright © 2010—2020 Sam Hocevar <sam@hocevar.net>
//
//  Lol Engine is free software. It comes without any warranty, to
//  the extent permitted by applicable law. You can redistribute it
//  and/or modify it under the terms of the Do What the Fuck You Want
//  to Public License, Version 2, as published by the WTFPL Task Force.
//  See http://www.wtfpl.net/ for more details.
//

#pragma once

//
// The Random number generators
// ————————————————————————————
//

#include <cassert>
#include <random>
#include <stdint.h>

namespace lol
{

// Random number generators
template<typename T = int>
[[nodiscard]] static inline T rand();

template<typename T>
[[nodiscard]] static inline T rand(T a);

template<typename T>
[[nodiscard]] static inline T rand(T a, T b);

// Two-value random number generator -- no need for specialisation
template<typename T> [[nodiscard]] static inline T rand(T a, T b)
{
    return a + rand<T>(b - a);
}

// One-value random number generators
template<typename T> [[nodiscard]] static inline T rand(T a)
{
    return a ? rand<T>() % a : T(0);
}

#if 0
template<> [[nodiscard]] inline half rand<half>(half a)
{
    return rand(1 << 13) / float(1 << 13) * a;
}
#endif

template<> [[nodiscard]] inline float rand<float>(float a)
{
    return rand(uint32_t(1) << 23) / float(uint32_t(1) << 23) * a;
}

template<> [[nodiscard]] inline double rand<double>(double a)
{
    return rand(uint64_t(1) << 53) / double(uint64_t(1) << 53) * a;
}

template<> [[nodiscard]] inline long double rand<long double>(long double a)
{
    return rand(uint64_t(1) << 63) / (long double)(uint64_t(1) << 63) * a;
}

// Default random number generator
template<typename T> [[nodiscard]] static inline T rand()
{
    static std::minstd_rand eng { std::random_device{}() };

    if constexpr (sizeof(T) == 1)
        return static_cast<T>(eng() & 0x7fu);

    if constexpr (sizeof(T) == 2)
        return static_cast<T>(eng() & 0x7fffu);

    if constexpr (sizeof(T) == 4)
        return static_cast<T>(eng() & 0x7fffffffu);

    if constexpr (sizeof(T) == 8)
    {
        uint64_t ret = eng();
        ret = (ret << 16) ^ eng();
        ret = (ret << 16) ^ eng();
        return static_cast<T>(ret & (~(uint64_t)0 >> 1));
    }

    assert(false);
}

#if 0
template<> [[nodiscard]] inline half rand<half>() { return rand<half>(1.f); }
#endif
template<> [[nodiscard]] inline float rand<float>() { return rand<float>(1.f); }
template<> [[nodiscard]] inline double rand<double>() { return rand<double>(1.0); }
template<> [[nodiscard]] inline long double rand<long double>() { return rand<long double>(1.0); }

} // namespace lol

