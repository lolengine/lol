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

#include <cmath> // std::floor
#include <functional> // std::function
#include <limits> // std::numeric_limits
#include <memory> // std::shared_ptr
#include <optional> // std::optional
#include <type_traits> // std::is_same_v
#include <unordered_set> // std::unordered_set

#include "../math/interp.h" // lol::interp::lanczos

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
        constexpr auto to_fp = std::is_floating_point_v<TO>;

        if constexpr (std::is_same_v<FROM, TO> || (from_fp && to_fp))
        {
            // If types are the same, or both floating point, no conversion is needed
            return TO(x);
        }
        else if constexpr (from_fp)
        {
            // From floating point to integer:
            //  - change range from -1…1 to 0…1
            //  - multiply by the size of the integer range
            //  - add min, round down, and clamp to min…max
            FROM constexpr min(std::numeric_limits<TO>::min());
            FROM constexpr max(std::numeric_limits<TO>::max());
            x = (max - min + 1) / 2 * (x + 1);
            return TO(std::max(min, std::min(max, std::floor(x + min))));
        }
        else if constexpr (to_fp)
        {
            // From integer to floating point:
            //  - compute (x - min) / (max - min)
            //  - change range from 0…1 to -1…1
            TO constexpr min(std::numeric_limits<FROM>::min());
            TO constexpr max(std::numeric_limits<FROM>::max());
            return 2 / (max - min) * (TO(x) - min) - 1;
        }
        else
        {
            // When converting between integer types, we first convert to an unsigned
            // type of same size as source (e.g. int16_t → uint16_t) to ensure that all
            // operations will happen modulo n (not guaranteed with signed types).
            // The next step is to shift right (drop bits) or promote left (multiply by
            // a magic constant such as 0x1010101 or 0x10001). This happens using the
            // UBIG type, which is an unsigned integer type at least as large as FROM
            // and TO.
            // Finally, we convert back to signed (e.g. uint16_t → int16_t) if necessary.
            using UFROM = std::make_unsigned_t<FROM>;
            using UTO = std::make_unsigned_t<TO>;
            using UBIG = std::conditional_t<(sizeof(FROM) > sizeof(TO)), UFROM, UTO>;

            UBIG constexpr mul = std::numeric_limits<UBIG>::max() / std::numeric_limits<UFROM>::max();
            UBIG constexpr div = UBIG(1) << 8 * (sizeof(UBIG) - sizeof(UTO));
            auto tmp = UFROM(UFROM(x) - UFROM(std::numeric_limits<FROM>::min())) * mul / div;

            return TO(tmp + UTO(std::numeric_limits<TO>::min()));
        }
    }

    // Saturated addition for samples
    template<typename T>
    static inline T sadd(T x, T y)
    {
        if constexpr (std::is_floating_point_v<T>)
        {
            // No saturation for floating point types
            return x + y;
        }
        else if constexpr (sizeof(T) <= 4)
        {
            // For integer types up to 32-bit, do the computation with a larger type
            using BIG = std::conditional_t<sizeof(T) == 1, int16_t,
                        std::conditional_t<sizeof(T) == 2, int32_t,
                        std::conditional_t<sizeof(T) == 4, int64_t, void>>>;
            BIG constexpr min = std::numeric_limits<T>::min();
            BIG constexpr max = std::numeric_limits<T>::max();
            BIG constexpr zero = (min + max + 1) >> 1;

            return T(std::max(min, std::min(max, BIG(BIG(x) + BIG(y) - zero))));
        }
        else if constexpr (std::is_unsigned_v<T>)
        {
            // Unsigned saturated add for 64-bit and larger: clamp according to overflow
            T constexpr zero = T(1) << 8 * sizeof(T) - 1;
            T constexpr minus_one = zero - T(1);
            T ret = x + y;
            return ret >= x ? std::max(zero, ret) - zero : std::min(minus_one, ret) + zero;
        }
        else
        {
            // Signed saturated add for 64-bit and larger: if signs differ, no overflow
            // occurred, just return the sum of the arguments; otherwise, clamp according
            // to the arguments sign.
            using U = std::make_unsigned_t<T>;
            U constexpr umax = U(std::numeric_limits<T>::max());
            U constexpr umin = U(std::numeric_limits<T>::min());
            U ret = U(x) + U(y);

            return T(x ^ y) < 0 ? T(ret) : x >= 0 ? T(std::min(ret, umax)) : T(std::max(ret, umin));
        }
    }

    // Clipping for samples
    template<typename T>
    static inline T clip(T x)
    {
        if constexpr (std::is_floating_point_v<T>)
        {
            return std::min(T(1), std::max(T(-1), x));
        }
        else
        {
            // Clipping is only relevant for floating point types
            return x;
        }
    }

    template<typename T>
    static inline T softclip(T x)
    {
        if constexpr (std::is_floating_point_v<T>)
        {
            return std::tanh(x);
        }
        else
        {
            // Clipping is only relevant for floating point types
            return x;
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

    virtual std::optional<size_t> size() const { return std::nullopt; }

    virtual std::optional<size_t> pos() const { return std::nullopt; }

    virtual bool seek(size_t pos) { return false; }

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
            T x = T(0);

            for (auto const &b : buffers)
                x = sample::sadd(x, b[n]);

            buf[n] = x;
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

    virtual std::optional<size_t> size() const override
    {
        return m_in->size();
    }

    virtual std::optional<size_t> pos() const override
    {
        return m_in->pos();
    }

    virtual bool seek(size_t pos) override
    {
        return m_in->seek(pos);
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
                T x(0);

                for (size_t in_ch = 0; in_ch < m_in->channels(); ++in_ch)
                    x = sample::sadd(x, tmp[f * m_in->channels() + in_ch] * matrix[out_ch * m_in->channels() + in_ch]);

                buf[f * this->channels() + out_ch] = x;
            }
        }

        return frames;
    }

    virtual std::optional<size_t> size() const override
    {
        return m_in->size();
    }

    virtual std::optional<size_t> pos() const override
    {
        return m_in->pos();
    }

    virtual bool seek(size_t pos) override
    {
        return m_in->seek(pos);
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
            while (m_cache.size() / channels < m_pos / out_rate + m_lanczos.size())
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
                *buf++ = m_lanczos.get(&m_cache[n0 * channels + ch], channels, alpha);
        }

        return frames;
    }

    // FIXME: size/pos/seek API

protected:
    std::shared_ptr<stream<T>> m_in;

    interp::lanczos<T> m_lanczos;

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
