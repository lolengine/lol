//
//  Lol Engine
//
//  Copyright © 2004—2020 Sam Hocevar <sam@hocevar.net>
//
//  Lol Engine is free software. It comes without any warranty, to
//  the extent permitted by applicable law. You can redistribute it
//  and/or modify it under the terms of the Do What the Fuck You Want
//  to Public License, Version 2, as published by the WTFPL Task Force.
//  See http://www.wtfpl.net/ for more details.
//

#pragma once

//
// The Pixel-related classes
// —————————————————————————
//

#include <lol/vector>

namespace lol::pixel
{

// The pixel formats we know about
enum class format : uint8_t
{
    y_u8, rgb_u8, rgba_u8, y_f32, rgb_f32, rgba_f32,
};

// Associated storage types for each pixel format
template<format T> struct type {};
template<> struct type<format::y_u8> { typedef uint8_t value; };
template<> struct type<format::rgb_u8> { typedef u8vec3 value; };
template<> struct type<format::rgba_u8> { typedef u8vec4 value; };
template<> struct type<format::y_f32> { typedef float value; };
template<> struct type<format::rgb_f32> { typedef vec3 value; };
template<> struct type<format::rgba_f32> { typedef vec4 value; };

// Convert between pixel formats
template<format T, format U>
typename type<U>::value convert(typename type<T>::value const &p)
{
    return type<T>::value();
}

} // namespace lol::pixel
