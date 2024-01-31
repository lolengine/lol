//
//  Lol Engine
//
//  Copyright © 2010–2024 Sam Hocevar <sam@hocevar.net>
//            © 2013–2015 Benjamin “Touky” Huet <huet.benjamin@gmail.com>
//
//  Lol Engine is free software. It comes without any warranty, to
//  the extent permitted by applicable law. You can redistribute it
//  and/or modify it under the terms of the Do What the Fuck You Want
//  to Public License, Version 2, as published by the WTFPL Task Force.
//  See http://www.wtfpl.net/ for more details.
//

#pragma once

//
// The string tools
// ————————————————
// Contains some utilities to work with std::string objects.
//

#include "../features.h"

#include <vector>    // std::vector
#include <string>    // std::basic_string
#include <algorithm> // std::transform
#include <iterator>  // std::back_inserter
#include <cctype>    // size_t

namespace lol
{

// Split a string along a single separator
template<typename T>
std::vector<std::basic_string<T>> split(std::basic_string<T> const &s,
                                        T sep = T('\n'))
{
    std::vector<std::basic_string<T>> ret;
    size_t start = 0, end = 0;
    while ((end = s.find(sep, start)) != std::basic_string<T>::npos)
    {
        ret.push_back(s.substr(start, end - start));
        start = end + 1;
    }
    ret.push_back(s.substr(start));
    return ret;
}

// Split a string along multiple separator characters
template<typename T>
std::vector<std::basic_string<T>> split(std::basic_string<T> const &s,
                                        std::basic_string<T> const &seps)
{
    std::vector<std::string> ret;
    size_t start = s.find_first_not_of(seps), end = 0;

    while ((end = s.find_first_of(seps, start)) != std::basic_string<T>::npos)
    {
        ret.push_back(s.substr(start, end - start));
        start = s.find_first_not_of(seps, end);
    }
    if (start != std::string::npos)
        ret.push_back(s.substr(start));

    return ret;

}

// Helper for template deduction
template<typename T>
std::vector<std::basic_string<T>> split(std::basic_string<T> const &s,
                                        T const *seps)
{
    return split(s, std::basic_string<T>(seps));
}

// Check whether a string starts with a given substring
template<typename T>
bool starts_with(std::basic_string<T> const &s,
                 std::basic_string<T> const &prefix)
{
    return s.size() >= prefix.size() &&
           s.compare(0, prefix.size(), prefix) == 0;
}

template<typename T>
bool starts_with(std::basic_string<T> const &s, T const *prefix)
{
    return starts_with(s, std::basic_string<T>(prefix));
}

template<typename T>
bool starts_with(T const *s, T const *suffix)
{
    return starts_with(std::basic_string<T>(s), std::basic_string<T>(suffix));
}

// Check whether a string ends with a given substring
template<typename T>
bool ends_with(std::basic_string<T> const &s,
               std::basic_string<T> const &suffix)
{
    return s.size() >= suffix.size() &&
           s.compare(s.size() - suffix.size(), suffix.size(), suffix) == 0;
}

template<typename T>
bool ends_with(std::basic_string<T> const &s, T const *suffix)
{
    return ends_with(s, std::basic_string<T>(suffix));
}

template<typename T>
bool ends_with(T const *s, T const *suffix)
{
    return ends_with(std::basic_string<T>(s), std::basic_string<T>(suffix));
}

// Convert a string to lowercase or uppercase
template<typename T>
std::basic_string<T> tolower(std::basic_string<T> const &s)
{
    std::string ret;
    std::transform(s.begin(), s.end(), std::back_inserter(ret),
                   [](T c){ return std::tolower(c); });
    return ret;
}

template<typename T>
std::basic_string<T> tolower(T const *s)
{
    return tolower(std::basic_string<T>(s));
}

template<typename T>
std::basic_string<T> toupper(std::basic_string<T> const &s)
{
    std::string ret;
    std::transform(s.begin(), s.end(), std::back_inserter(ret),
                   [](T c){ return std::toupper(c); });
    return ret;
}

template<typename T>
std::basic_string<T> toupper(T const *s)
{
    return toupper(std::basic_string<T>(s));
}

} // namespace lol
