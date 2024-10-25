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
// The image class
// ———————————————
//

#include <lol/narray> // lol::array2d
#include <lol/vector> // lol::ivec2

namespace lol::image
{

enum class EdiffAlgorithm : uint8_t
{
    FloydSteinberg,
    JaJuNi,
    Atkinson,
    Fan,
    ShiauFan,
    ShiauFan2,
    Stucki,
    Burkes,
    Sierra,
    Sierra2,
    Lite,
};

struct kernel
{
    kernel() = delete;

    static array2d<float> normalize(array2d<float> const &kernel);

    static array2d<float> bayer(ivec2 size);
    static array2d<float> halftone(ivec2 size);
    static array2d<float> blue_noise(ivec2 size,
                                     ivec2 gsize = ivec2(7, 7));
    static array2d<float> ediff(EdiffAlgorithm algorithm);
    static array2d<float> gaussian(vec2 radius,
                                   float angle = 0.f,
                                   vec2 delta = vec2(0.f, 0.f));
};

} // namespace lol::image

#include "kernel.ipp"
