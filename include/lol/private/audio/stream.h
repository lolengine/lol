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
// The audio stream interface
// ——————————————————————————
// Stream, mix, and apply audio effects.
//

#include <functional> // std::function
#include <memory> // std::shared_ptr
#include <type_traits> // std::is_same_v
#include <unordered_set> // std::unordered_set

namespace lol::audio
{

template<typename T>
class stream
{
public:
    using sample_type = T;

    virtual size_t get(T* buf, size_t frames) = 0;

    size_t channels() const { return m_channels; }

    int frequency() const { return m_frequency; }

    virtual ~stream() = default;

protected:
    stream(size_t channels, int frequency)
      : m_channels(channels),
        m_frequency(frequency)
    {}

    size_t m_channels;
    int m_frequency;
};

template<typename T>
class generator : public stream<T>
{
public:
    generator(std::function<size_t(T*, size_t)> get, size_t channels, int frequency)
        : stream<T>(channels, frequency),
          m_get(get)
    {}

    virtual size_t get(T* buf, size_t frames) override
    {
        return m_get(buf, frames);
    }

protected:
    std::function<size_t(T*, size_t)> m_get;
};

template<typename T>
class mixer : public stream<T>
{
public:
    mixer(size_t channels, int frequency)
      : stream<T>(channels, frequency)
    {}

    void add(std::shared_ptr<stream<T>> s)
    {
        // FIXME: check the channel count!
        m_streamers.insert(s);
    }

    void remove(std::shared_ptr<stream<T>> s)
    {
        m_streamers.erase(s);
    }

    virtual size_t get(T *buf, size_t frames) override
    {
        memset(buf, 0, frames * this->channels() * sizeof(T));

        std::vector<T> tmp(frames * this->channels());
        for (auto s : m_streamers)
        {
            s->get(tmp.data(), frames);
            for (size_t i = 0; i < tmp.size(); ++i)
                buf[i] += tmp[i];
        }

        return frames;
    }

protected:
    std::unordered_set<std::shared_ptr<stream<T>>> m_streamers;
};

template<typename T0, typename T>
static inline T convert_sample(T0);

template<>
inline float convert_sample(float x) { return x; }

template<>
inline float convert_sample(int16_t x) { return x / 32768.0f; }

template<>
inline float convert_sample(uint16_t x) { return x / 32767.0f - 1.0f; }

template<typename T, typename T0>
class converter : public stream<T>
{
public:
    converter(std::shared_ptr<stream<T0>> s, size_t channels)
      : stream<T>(channels, s->frequency()),
        s0(s)
    {}

    virtual size_t get(T *buf, size_t frames) override
    {
        if constexpr(std::is_same_v<T0, T>)
            if (this->channels() == s0->channels())
                return s0->get(buf, frames);

        std::vector<T0> tmp(frames * s0->channels());
        s0->get(tmp.data(), frames);
        for (size_t f = 0; f < frames; ++f)
        {
            buf[f * this->channels()] = tmp[f * s0->channels()];
            for (size_t ch = 1; ch < this->channels(); ++ch)
            {
                size_t ch0 = std::max(size_t(0), s0->channels() - (this->channels() - ch));
                buf[f * this->channels() + ch] = convert_sample<T0, T>(tmp[f * s0->channels() + ch0]);
            }
        }

        return frames;
    }

protected:
    std::shared_ptr<stream<T0>> s0;
};

template<typename T>
class resampler : public stream<T>
{
public:
    resampler(std::shared_ptr<stream<T>> s, int frequency)
      : stream<T>(s->channels(), frequency),
        s(s)
    {}

    virtual size_t get(T *buf, size_t frames) override
    {
        if (s->frequency() == this->frequency())
            return s->get(buf, frames);

        // FIXME: implement resampling!
        memset(buf, 0, frames * this->channels() * sizeof(T));
        return frames;
    }

protected:
    std::shared_ptr<stream<T>> s;
};

template<typename T>
static inline auto make_generator(std::function<size_t(T*, size_t)> f, size_t channels, int frequency)
{
    return std::make_shared<generator<T>>(f, channels, frequency);
}

template<typename F>
static inline auto make_generator(F f, size_t channels, int frequency)
{
    return make_generator(std::function(f), channels, frequency);
}

template<typename T, typename S0, typename T0 = S0::sample_type>
static inline auto make_converter(std::shared_ptr<S0> s, size_t channels)
{
    return std::make_shared<converter<T, T0>>(std::shared_ptr<stream<T0>>(s), channels);
}

template<typename S, typename T = S::sample_type>
static inline auto make_resampler(std::shared_ptr<S> s, int frequency)
{
    return std::make_shared<resampler<T>>(std::shared_ptr<stream<T>>(s), frequency);
}

template<typename T, typename S0, typename T0 = S0::sample_type>
static inline auto make_adapter(std::shared_ptr<S0> s, size_t channels, int frequency)
{
    return make_resampler(make_converter<T>(s, channels), frequency);
}

} // namespace lol::audio
