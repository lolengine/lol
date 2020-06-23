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
// The file-related classes
// ————————————————————————
// These do not use std::filesystem yet because of the stdc++fs link requirement.
//

#include <fstream> // std::ofstream

namespace lol::file
{

template<typename T, typename U = typename T::value_type>
static inline bool read(std::string const &path, T &data)
{
    std::ifstream f(path, std::ios::in | std::ios::binary | std::ios::ate);
    auto file_size = f.tellg(); // works because std::ios::ate
    if (file_size < 0)
        return false;
    f.seekg(0, std::ios::beg);
    data.resize((size_t(file_size) + sizeof(U) - 1) / sizeof(U));
    f.read((char *)data.data(), file_size);
    return !f.fail();
}

template<typename T, typename U = typename T::value_type>
static inline bool write(std::string const &path, T const &data)
{
    std::ofstream f(path, std::ios::binary);
    f.write((char const *)data.data(), data.size() * sizeof(U));
    f.close();
    return !f.fail();
}

} // namespace lol::file
