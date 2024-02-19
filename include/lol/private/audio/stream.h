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
#include <limits> // std::numeric_limits
#include <memory> // std::shared_ptr
#include <type_traits> // std::is_same_v
#include <unordered_set> // std::unordered_set

namespace lol::audio
{

class sample
{
public:
    // Convert samples between different types (float, int16_t, uint8_t, …)
    template<typename FROM, typename TO>
    static inline TO convert(FROM x)
    {
        constexpr auto from_fp = std::is_floating_point_v<FROM>;
        constexpr auto from_signed = !from_fp && std::is_signed_v<FROM>;
        constexpr auto to_fp = std::is_floating_point_v<TO>;
        constexpr auto to_signed = !to_fp && std::is_signed_v<TO>;

        if constexpr (std::is_same_v<FROM, TO> || (from_fp && to_fp))
        {
            // If types are the same, or both floating-point, no conversion is needed
            return TO(x);
        }
        else if constexpr (from_fp)
        {
            // From floating point to integer:
            //  - renormalise to 0…1
            //  - multiply by the size of the integer range
            //  - add min, round down, and clamp to min…max
            FROM constexpr min(std::numeric_limits<TO>::min());
            FROM constexpr max(std::numeric_limits<TO>::max());
            x = (x + 1) * ((max - min + 1) / 2);
            return TO(std::max(min, std::min(max, std::floor(x + min))));
        }
        else if constexpr (to_fp)
        {
            TO constexpr min(std::numeric_limits<FROM>::min());
            TO constexpr max(std::numeric_limits<FROM>::max());
            return (TO(x) - min) * 2 / (max - min) - 1;
        }
        else
        {
            // FIXME: this is better than nothing but we need a better implementation
            return convert<double, TO>(convert<FROM, double>(x));
        }
    }
};

template<typename T>
class stream
{
public:
    using sample_type = T;

    virtual size_t get(T* buf, size_t frames) = 0;

    inline size_t channels() const { return m_channels; }

    inline int frequency() const { return m_frequency; }

    inline size_t frame_size() const { return channels() * sizeof(T); }

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
        m_streams.insert(s);
    }

    void remove(std::shared_ptr<stream<T>> s)
    {
        m_streams.erase(s);
    }

    virtual size_t get(T *buf, size_t frames) override
    {
        std::vector<std::vector<T>> buffers;
        size_t const samples = frames * this->channels();

        for (auto s : m_streams)
        {
            buffers.push_back(std::vector<T>(samples));
            s->get(buffers.back().data(), frames);
        }

        for (size_t n = 0; n < samples; ++n)
        {
            T sample = T(0);
            for (auto const &b : buffers)
                sample += b[n];

            if constexpr (std::is_same_v<T, float>)
                buf[n] = std::min(1.0f, std::max(-1.0f, sample));
            else if constexpr (std::is_same_v<T, int16_t>)
                buf[n] = std::min(int16_t(32767), std::max(int16_t(-32768), sample));
            else if constexpr (std::is_same_v<T, uint16_t>)
                buf[n] = std::min(uint16_t(65535), std::max(uint16_t(0), sample));
            else
                buf[n] = sample;
        }

        return frames;
    }

protected:
    std::unordered_set<std::shared_ptr<stream<T>>> m_streams;
};

template<typename T, typename T0>
class converter : public stream<T>
{
public:
    converter(std::shared_ptr<stream<T0>> s)
      : stream<T>(s->channels(), s->frequency()),
        m_in(s)
    {}

    virtual size_t get(T *buf, size_t frames) override
    {
        if constexpr (std::is_same_v<T0, T>)
            return m_in->get(buf, frames);

        size_t samples = frames * this->channels();

        std::vector<T0> tmp(samples);
        m_in->get(tmp.data(), frames);
        for (size_t n = 0; n < samples; ++n)
            buf[n] = sample::convert<T0, T>(tmp[n]);

        return frames;
    }

protected:
    std::shared_ptr<stream<T0>> m_in;
};

template<typename T>
class mapper : public stream<T>
{
public:
    mapper(std::shared_ptr<stream<T>> s, size_t channels)
      : stream<T>(channels, s->frequency()),
        m_in(s)
    {}

    virtual size_t get(T *buf, size_t frames) override
    {
        if (this->channels() == m_in->channels())
            return m_in->get(buf, frames);

        std::vector<T> tmp(frames * m_in->channels());
        m_in->get(tmp.data(), frames);

        // FIXME: we need to build a better matrix than this, also maybe the mapper
        // constructor needs an option to preserve energy or not?
        std::vector<T> matrix(m_in->channels() * this->channels(), T(1));

        for (size_t f = 0; f < frames; ++f)
        {
            for (size_t out_ch = 0; out_ch < this->channels(); ++out_ch)
            {
                T sample(0);
                for (size_t in_ch = 0; in_ch < m_in->channels(); ++in_ch)
                    sample += tmp[f * m_in->channels() + in_ch] * matrix[out_ch * m_in->channels() + in_ch];

                if constexpr (std::is_same_v<T, float>)
                    buf[f * this->channels() + out_ch] = std::min(1.0f, std::max(-1.0f, sample));
                else if constexpr (std::is_same_v<T, int16_t>)
                    buf[f * this->channels() + out_ch] = std::min(int16_t(32767), std::max(int16_t(-32768), sample));
                else if constexpr (std::is_same_v<T, uint16_t>)
                    buf[f * this->channels() + out_ch] = std::min(uint16_t(65535), std::max(uint16_t(0), sample));
                else
                    buf[f * this->channels() + out_ch] = sample;
            }
        }

        return frames;
    }

protected:
    std::shared_ptr<stream<T>> m_in;
};

template<typename T>
class resampler : public stream<T>
{
public:
    resampler(std::shared_ptr<stream<T>> s, int frequency)
      : stream<T>(s->channels(), frequency),
        m_in(s),
        m_pos(0)
    {}

    virtual size_t get(T *buf, size_t frames) override
    {
        size_t const channels = this->channels();
        size_t const in_rate = m_in->frequency();
        size_t const out_rate = this->frequency();

        if (in_rate == out_rate)
            return m_in->get(buf, frames);

        for (size_t n = 0; n < frames; ++n, m_pos += in_rate)
        {
            // Fill internal buffer if we don’t have enough data
            while (m_cache.size() / channels < m_pos / out_rate + 2)
            {
                // Remove obsolete frames on the left
                size_t todelete = std::min(m_pos / out_rate, m_cache.size() / channels);
                std::vector<T>(m_cache.begin() + todelete * channels, m_cache.end()).swap(m_cache);
                m_pos -= todelete * out_rate;

                // Add new frames to the right
                size_t offset = m_cache.size();
                m_cache.resize(offset + frames * channels);
                m_in->get(&m_cache[offset], frames);
            }

            size_t n0 = m_pos / out_rate;
            float alpha = float(m_pos % out_rate) / out_rate;

            for (size_t ch = 0; ch < channels; ++ch)
            {
                *buf++ = m_cache[n0 * channels + ch] * (1.f - alpha)
                       + m_cache[(n0 + 1) * channels + ch] * alpha;
            }
        }

        return frames;
    }

protected:
    std::shared_ptr<stream<T>> m_in;

    std::vector<T> m_cache;

    size_t m_pos;
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

template<typename T, typename S0, typename T0 = typename S0::sample_type>
static inline auto make_converter(std::shared_ptr<S0> s)
{
    return std::make_shared<converter<T, T0>>(std::shared_ptr<stream<T0>>(s));
}

template<typename S, typename T = typename S::sample_type>
static inline auto make_mapper(std::shared_ptr<S> s, size_t channels)
{
    return std::make_shared<mapper<T>>(std::shared_ptr<stream<T>>(s), channels);
}

template<typename S, typename T = typename S::sample_type>
static inline auto make_resampler(std::shared_ptr<S> s, int frequency)
{
    return std::make_shared<resampler<T>>(std::shared_ptr<stream<T>>(s), frequency);
}

template<typename T, typename S0, typename T0 = typename S0::sample_type>
static inline auto make_adapter(std::shared_ptr<S0> s, size_t channels, int frequency)
{
    return make_resampler(make_mapper(make_converter<T>(s), channels), frequency);
}

} // namespace lol::audio
