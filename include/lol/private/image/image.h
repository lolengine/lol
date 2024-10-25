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
// The image class
// ———————————————
//

#include "pixel.h"

#include <lol/narray> // lol::array2d
#include <lol/vector>    // lol::ivec2
#include <memory>        // std::shared_ptr
#include <string>        // std::string
#include <unordered_set> // std::unordered_set

namespace lol
{

template<pixel::format T> class image_t;

using image = image_t<pixel::format::rgba_u8>;

class image_data
{
    virtual ~image_data() {}
};

// Codec class for loading and saving images
class image_codec
{
public:
    virtual bool test(std::string const &path) = 0;
    virtual bool load(std::string const &path, image &im) = 0;
    virtual bool save(std::string const &path, image &im) = 0;

    static void add(std::shared_ptr<image_codec> codec)
    {
        codecs().insert(codec);
    }

    static std::unordered_set<std::shared_ptr<class image_codec>> codecs()
    {
        static std::unordered_set<std::shared_ptr<image_codec>> instance;
        return instance;
    };
};

// The generic image class
template<pixel::format T> class image_t
{
public:
    using pixel_type = typename pixel::type<T>::value;

    image_t<T>() = default;

    image_t<T>(int width, int height)
      : m_pixels(width, height)
    {}

    image_t<T>(ivec2 size)
      : m_pixels(size)
    {}

    // Build from another image
    template<pixel::format U>
    image_t<T>(image_t<U> const &other)
      : m_pixels(other.size)
    {
        auto src = other.pixels();
        auto dst = pixels();

        for (size_t i = 0; i < src.size(); ++i)
            dst[i] = pixel::convert<U, T>(src[i]);
    }

    // Load and save image
    bool load(std::string const &path)
    {
        for (auto &codec : image_codec::codecs())
            if (codec->load(path, *this))
                 return true;
        return false;
    }

    bool save(std::string const &path)
    {
        for (auto &codec : image_codec::codecs())
            if (codec->save(path, *this))
                 return true;
        return false;
    }

    // Pixel array properties
    ivec2 size() const { return m_pixels.sizes(); }
    size_t bytes() const { return m_pixels.bytes(); }

    // Direct access to pixels
    span2d<pixel_type> pixels()
    {
        return m_pixels.span();
    }

    span2d<pixel_type const> pixels() const
    {
        return m_pixels.span();
    }

private:
    array2d<pixel_type> m_pixels;
    std::shared_ptr<image_data> m_data;
};

using image = image_t<pixel::format::rgba_u8>;

enum class WrapMode : uint8_t
{
    Clamp,
    Repeat,
};

enum class ScanMode : uint8_t
{
    Raster,
    Serpentine,
};

enum class ResampleAlgorithm : uint8_t
{
    Bicubic,
    Bresenham,
};

} // namespace lol
