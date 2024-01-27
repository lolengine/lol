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

#if !_DEBUG
#include "../base/env.h" // lol::sys::getenv
#endif

#include <cstdarg> // va_start
#include <cstdio> // std::vfprintf
#include <functional> // std::function
#include <string> // std::string

namespace lol
{

struct msg
{
    template<typename... T>
    static inline void debug(char const *fmt, T... args)
    {
        helper(message_type::debug, fmt, args...);
    }

    template<typename... T>
    static inline void info(char const *fmt, T... args)
    {
        helper(message_type::info, fmt, args...);
    }

    template<typename... T>
    static inline void warn(char const *fmt, T... args)
    {
        helper(message_type::warn, fmt, args...);
    }

    template<typename... T>
    static inline void error(char const *fmt, T... args)
    {
        helper(message_type::error, fmt, args...);
    }

    // Allow the user to provide their own logging function
    static void set_output(std::function<bool(std::string const&)> fn)
    {
        get_output() = fn;
    }

private:
    msg() = delete;

    enum class message_type
    {
        debug,
        info,
        warn,
        error,
    };

    static std::function<bool(std::string const&)>& get_output()
    {
        static std::function<bool(std::string const&)> output;
        return output;
    };

    static void helper(message_type type, char const *fmt, ...)
    {
        // Unless this is a debug build, ignore debug messages unless
        // the LOL_DEBUG environment variable is set.
#if !_DEBUG
        if (type == message_type::debug)
        {
            static bool const enforce_debug = sys::getenv("LOL_DEBUG").size() > 0;
            if (!enforce_debug)
                return;
        }
#endif

        static std::string prefix[]
        {
            "DEBUG",
            "INFO",
            "WARN",
            "ERROR",
        };

        std::string str = prefix[int(type)] + ": ";
        size_t pos = str.size();

        va_list ap;
        va_start(ap, fmt);
        size_t count = std::vsnprintf(nullptr, 0, fmt, ap);
        str.resize(pos + count);
        // This is OK because C++ strings are indeed null-terminated
        std::vsnprintf(str.data() + pos, count + 1, fmt, ap);
        va_end(ap);

        // If a user function was provided, use it, and if it returns true, stop
        // processing the message immediately.
        if (auto &fn = get_output(); fn && fn(str))
            return;

        fprintf(stderr, "%s", str.c_str());
        fflush(stderr);
    }
};

} // namespace lol
