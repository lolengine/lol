//
//  Lol Engine
//
//  Copyright © 2010–2023 Sam Hocevar <sam@hocevar.net>
//
//  Lol Engine is free software. It comes without any warranty, to
//  the extent permitted by applicable law. You can redistribute it
//  and/or modify it under the terms of the Do What the Fuck You Want
//  to Public License, Version 2, as published by the WTFPL Task Force.
//  See http://www.wtfpl.net/ for more details.
//

#pragma once

//
// The vector classes
// ——————————————————
//

#include <cassert>
#include <ostream>     // std::ostream
#include <type_traits>
#include <algorithm>   // std::min, std::max
#include <cmath>       // std::fabs, std::cos…

// FIXME: get rid of this, too
#include <../legacy/lol/base/types.h>

#include "ops.h"

namespace lol
{

template<typename T> struct quat_t;

/*
 * Magic vector swizzling
 *
 * These vectors are empty, but thanks to static_cast we can take their
 * address and access the vector of T's that they are union'ed with. We
 * use static_cast instead of reinterpret_cast because there is a stronger
 * guarantee (by the standard) that the address will stay the same across
 * casts.
 *
 * Using swizzled vectors as lvalues:
 * We need to implement an assignment operator _and_ override the default
 * assignment operator. We try to pass arguments by value so that we don't
 * have to care about overwriting ourselves (e.g. c.rgb = c.bgr).
 * However, Visual Studio 2013 doesn't support unrestricted unions, so
 * fuck it for now.
 */

template<typename T, int N, int SWIZZLE>
struct [[nodiscard]] vec_t
    /* MUST have a different base than e.g. vec_t<T,2> otherwise the unions
     * in vec_t<T,2> with the same base will cause empty base optimisation
     * failures. */
  : public swizzle_ops::base<T, SWIZZLE>
{
    static int const count = N;
    typedef T scalar_element;
    typedef T element;
    typedef vec_t<T,N> type;

    /* Allow the assignment operator if unrestricted unions are supported. */
    inline vec_t<T, N, SWIZZLE>& operator =(vec_t<T, N> that)
    {
        for (int i = 0; i < N; ++i)
            (*this)[i] = that[i];
        return *this;
    }

    inline vec_t<T, N, SWIZZLE>& operator =(vec_t<T, N, SWIZZLE> const &that)
    {
        /* Pass by value in case this == &that */
        return *this = (vec_t<T,N>)that;
    }

    inline T& operator[](size_t n)
    {
        int const i = (SWIZZLE >> (3 * (N - 1 - n))) & 3;
        return static_cast<T*>(static_cast<void*>(this))[i];
    }

    inline T const& operator[](size_t n) const
    {
        int const i = (SWIZZLE >> (3 * (N - 1 - n))) & 3;
        return static_cast<T const*>(static_cast<void const *>(this))[i];
    }

private:
    // Hide all default constructors and destructors; this object
    // is only intended to exist as part of a union.
    template<typename T2, int N2, int SWIZZLE2> friend struct vec_t;

    vec_t() = default;
    vec_t(vec_t<T, N, SWIZZLE> const &) = default;
    ~vec_t() = default;
};

/*
 * Helper macros for vector type member functions
 */

#define LOL_COMMON_MEMBER_OPS(first) \
    /* Use reinterpret_cast because static_cast is illegal here */ \
    inline T& operator[](size_t n) { return reinterpret_cast<T*>(this)[n]; } \
    inline T const& operator[](size_t n) const { return reinterpret_cast<T const*>(this)[n]; } \
    \
    /* An explicit assignment operator is now mandatory */ \
    inline type & operator =(type const &that) \
    { \
        for (int i = 0; i < type::count; ++i) \
            (*this)[i] = that[i]; \
        return *this; \
    } \
    \
    std::string tostring() const;

/* The generic “vec_t” type, which is a fixed-size vector with no
 * swizzling. There's an override for N=2, N=3, N=4 that has swizzling. */
template<typename T, int N>
struct [[nodiscard]] vec_t<T, N, FULL_SWIZZLE>
  : public componentwise_ops::base<T>
{
    static int const count = N;
    typedef T scalar_element;
    typedef T element;
    typedef vec_t<T,N> type;

    // Default constructor, copy constructor, and destructor
    inline constexpr vec_t() = default;
    inline constexpr vec_t(type const &v) = default;
    inline ~vec_t() = default;

    LOL_COMMON_MEMBER_OPS(m_data[0])

    /* Explicit constructor that takes exactly N arguments thanks to SFINAE. */
    template<typename... ARGS>
    explicit inline vec_t(T const &X,
        typename std::enable_if<sizeof...(ARGS) + 2 == N, T>::type const &Y,
        ARGS... args)
    {
        static_assert(sizeof...(ARGS) + 2 == N,
                      "wrong argument count in vec_t constructor");
        internal_init(m_data, X, Y, args...);
    }

    /* Various explicit constructors */
    explicit inline vec_t(T const &X)
    {
        for (auto &value : m_data)
            value = X;
    }

    /* Explicit constructor for type conversion */
    template<typename U>
    explicit inline vec_t(vec_t<U, N> const &v)
    {
        for (int i = 0; i < N; ++i)
            m_data[i] = T(v[i]);
    }

    /* Factory for base axis vectors, e.g. [1,0,0,…,0] */
    static inline vec_t<T, N> axis(int i)
    {
        assert(i >= 0);
        assert(i < N);
        vec_t<T, N> ret(T(0));
        ret[i] = T(1);
        return ret;
    }

    /* Constructor for initializer_list. We need these ugly
     * loops until C++ lets us initialize m_data directly. */
    inline vec_t(std::initializer_list<element> const &list)
    {
        auto l = list.begin();
        for (int i = 0; i < count && l != list.end(); ++i, ++l)
            m_data[i] = *l;
        for (int i = (int)list.size(); i < count; ++i)
            m_data[i] = T(0);
    }

    static const vec_t<T,N> zero;

private:
    template<typename... ARGS>
    static inline void internal_init(T *data, T const &x, ARGS... args)
    {
        *data++ = x;
        internal_init(data, args...);
    }

    static inline void internal_init(T *data)
    {
        (void)data;
    }

    T m_data[count];
};

/*
 * 2-element vectors
 */

template <typename T>
struct [[nodiscard]] vec_t<T,2>
  : public swizzle_ops::base<T>
{
    static int const count = 2;
    typedef T scalar_element;
    typedef T element;
    typedef vec_t<T,2> type;

    // Default constructor, copy constructor, and destructor
    inline constexpr vec_t() = default;
    inline constexpr vec_t(type const &v) = default;
    inline ~vec_t() = default;

    /* Implicit constructor for swizzling */
    template<int SWIZZLE>
    inline constexpr vec_t(vec_t<T, 2, SWIZZLE> const &v)
      : x(v[0]), y(v[1]) {}

    /* Explicit constructor for type conversion */
    template<typename U, int SWIZZLE>
    explicit inline constexpr vec_t(vec_t<U, 2, SWIZZLE> const &v)
      : x(T(v[0])), y(T(v[1])) {}

    /* Constructor for initializer_list. We need these ugly
     * loops until C++ lets us initialize m_data directly. */
    inline vec_t(std::initializer_list<element> const &list)
    {
        auto l = list.begin();
        for (int i = 0; i < count && l != list.end(); ++i, ++l)
            m_data[i] = *l;
        for (int i = (int)list.size(); i < count; ++i)
            m_data[i] = T(0);
    }

    /* Various explicit constructors */
    explicit inline constexpr vec_t(T X, T Y)
      : x(X), y(Y) {}
    explicit inline constexpr vec_t(T X)
      : x(X), y(X) {}

    /* Factory for base axis vectors, e.g. [1,0,0,…,0] */
    static inline vec_t<T,2> axis(int i)
    {
        assert(i >= 0);
        assert(i < 2);
        return vec_t<T,2>(T(i == 0), T(i == 1));
    }

    LOL_COMMON_MEMBER_OPS(x)

    static const vec_t<T,2> zero;
    static const vec_t<T,2> axis_x;
    static const vec_t<T,2> axis_y;

    union
    {
        struct { T x, y; }; /* axis */
        struct { T r, g; }; /* red, green */
        struct { T s, t; };

#if !_DOXYGEN_SKIP_ME
        vec_t<T,2,000> const xx, rr, ss;
        vec_t<T,2,001> const xy, rg, st;
        vec_t<T,2,010> const yx, gr, ts;
        vec_t<T,2,011> const yy, gg, tt;

        vec_t<T,3,0000> const xxx, rrr, sss;
        vec_t<T,3,0001> const xxy, rrg, sst;
        vec_t<T,3,0010> const xyx, rgr, sts;
        vec_t<T,3,0011> const xyy, rgg, stt;
        vec_t<T,3,0100> const yxx, grr, tss;
        vec_t<T,3,0101> const yxy, grg, tst;
        vec_t<T,3,0110> const yyx, ggr, tts;
        vec_t<T,3,0111> const yyy, ggg, ttt;

        vec_t<T,4,00000> const xxxx, rrrr, ssss;
        vec_t<T,4,00001> const xxxy, rrrg, ssst;
        vec_t<T,4,00010> const xxyx, rrgr, ssts;
        vec_t<T,4,00011> const xxyy, rrgg, sstt;
        vec_t<T,4,00100> const xyxx, rgrr, stss;
        vec_t<T,4,00101> const xyxy, rgrg, stst;
        vec_t<T,4,00110> const xyyx, rggr, stts;
        vec_t<T,4,00111> const xyyy, rggg, sttt;
        vec_t<T,4,01000> const yxxx, grrr, tsss;
        vec_t<T,4,01001> const yxxy, grrg, tsst;
        vec_t<T,4,01010> const yxyx, grgr, tsts;
        vec_t<T,4,01011> const yxyy, grgg, tstt;
        vec_t<T,4,01100> const yyxx, ggrr, ttss;
        vec_t<T,4,01101> const yyxy, ggrg, ttst;
        vec_t<T,4,01110> const yyyx, gggr, ttts;
        vec_t<T,4,01111> const yyyy, gggg, tttt;
#endif

        T m_data[count];
    };
};

/*
 * 3-element vectors
 */

template <typename T>
struct [[nodiscard]] vec_t<T,3>
  : public swizzle_ops::base<T>
{
    static int const count = 3;
    typedef T scalar_element;
    typedef T element;
    typedef vec_t<T,3> type;

    // Default constructor, copy constructor, and destructor
    inline constexpr vec_t() = default;
    inline constexpr vec_t(type const &v) = default;
    inline ~vec_t() = default;

    /* Implicit constructor for swizzling */
    template<int SWIZZLE>
    inline constexpr vec_t(vec_t<T, 3, SWIZZLE> const &v)
      : x(v[0]), y(v[1]), z(v[2]) {}

    /* Explicit constructor for type conversion */
    template<typename U, int SWIZZLE>
    explicit inline constexpr vec_t(vec_t<U, 3, SWIZZLE> const &v)
      : x(T(v[0])), y(T(v[1])), z(T(v[2])) {}

    /* Constructor for initializer_list. We need these ugly
     * loops until C++ lets us initialize m_data directly. */
    inline vec_t(std::initializer_list<element> const &list)
    {
        auto l = list.begin();
        for (int i = 0; i < count && l != list.end(); ++i, ++l)
            m_data[i] = *l;
        for (int i = (int)list.size(); i < count; ++i)
            m_data[i] = T(0);
    }

    /* Various explicit constructors */
    explicit inline constexpr vec_t(T X)
      : x(X), y(X), z(X) {}
    explicit inline constexpr vec_t(T X, T Y, T Z)
      : x(X), y(Y), z(Z) {}
    explicit inline constexpr vec_t(vec_t<T,2> XY, T Z)
      : x(XY.x), y(XY.y), z(Z) {}
    explicit inline constexpr vec_t(T X, vec_t<T,2> YZ)
      : x(X), y(YZ.x), z(YZ.y) {}

    /* Factory for base axis vectors, e.g. [1,0,0,…,0] */
    static inline vec_t<T,3> axis(int i)
    {
        assert(i >= 0);
        assert(i < 3);
        return vec_t<T,3>(T(i == 0), T(i == 1), T(i == 2));
    }

    LOL_COMMON_MEMBER_OPS(x)

    static vec_t<T,3> toeuler_xyx(quat_t<T> const &q);
    static vec_t<T,3> toeuler_xzx(quat_t<T> const &q);
    static vec_t<T,3> toeuler_yxy(quat_t<T> const &q);
    static vec_t<T,3> toeuler_yzy(quat_t<T> const &q);
    static vec_t<T,3> toeuler_zxz(quat_t<T> const &q);
    static vec_t<T,3> toeuler_zyz(quat_t<T> const &q);

    static vec_t<T,3> toeuler_xyz(quat_t<T> const &q);
    static vec_t<T,3> toeuler_xzy(quat_t<T> const &q);
    static vec_t<T,3> toeuler_yxz(quat_t<T> const &q);
    static vec_t<T,3> toeuler_yzx(quat_t<T> const &q);
    static vec_t<T,3> toeuler_zxy(quat_t<T> const &q);
    static vec_t<T,3> toeuler_zyx(quat_t<T> const &q);

    /* Return the cross product (vector product) of “a” and “b” */ \
    friend inline type cross(type const &a, type const &b)
    {
        return type(a.y * b.z - a.z * b.y,
                    a.z * b.x - a.x * b.z,
                    a.x * b.y - a.y * b.x);
    }

    /* Return a vector that is orthogonal to “a” */
    friend inline type orthogonal(type const &a)
    {
        using std::fabs;

        return fabs(a.x) > fabs(a.z)
             ? type(-a.y, a.x, T(0))
             : type(T(0), -a.z, a.y);
    }

    /* Return a vector that is orthonormal to “a” */
    friend inline type orthonormal(type const &a)
    {
        return normalize(orthogonal(a));
    }

    static const vec_t<T,3> zero;
    static const vec_t<T,3> axis_x;
    static const vec_t<T,3> axis_y;
    static const vec_t<T,3> axis_z;

    union
    {
        struct { T x, y, z; }; /* axis */
        struct { T r, g, b; }; /* red, green, blue */
        struct { T s, t, p; };

#if !_DOXYGEN_SKIP_ME
        vec_t<T,2,000> const xx, rr, ss;
        vec_t<T,2,001> const xy, rg, st;
        vec_t<T,2,002> const xz, rb, sp;
        vec_t<T,2,010> const yx, gr, ts;
        vec_t<T,2,011> const yy, gg, tt;
        vec_t<T,2,012> const yz, gb, tp;
        vec_t<T,2,020> const zx, br, ps;
        vec_t<T,2,021> const zy, bg, pt;
        vec_t<T,2,022> const zz, bb, pp;

        vec_t<T,3,0000> const xxx, rrr, sss;
        vec_t<T,3,0001> const xxy, rrg, sst;
        vec_t<T,3,0002> const xxz, rrb, ssp;
        vec_t<T,3,0010> const xyx, rgr, sts;
        vec_t<T,3,0011> const xyy, rgg, stt;
        vec_t<T,3,0012> const xyz, rgb, stp;
        vec_t<T,3,0020> const xzx, rbr, sps;
        vec_t<T,3,0021> const xzy, rbg, spt;
        vec_t<T,3,0022> const xzz, rbb, spp;
        vec_t<T,3,0100> const yxx, grr, tss;
        vec_t<T,3,0101> const yxy, grg, tst;
        vec_t<T,3,0102> const yxz, grb, tsp;
        vec_t<T,3,0110> const yyx, ggr, tts;
        vec_t<T,3,0111> const yyy, ggg, ttt;
        vec_t<T,3,0112> const yyz, ggb, ttp;
        vec_t<T,3,0120> const yzx, gbr, tps;
        vec_t<T,3,0121> const yzy, gbg, tpt;
        vec_t<T,3,0122> const yzz, gbb, tpp;
        vec_t<T,3,0200> const zxx, brr, pss;
        vec_t<T,3,0201> const zxy, brg, pst;
        vec_t<T,3,0202> const zxz, brb, psp;
        vec_t<T,3,0210> const zyx, bgr, pts;
        vec_t<T,3,0211> const zyy, bgg, ptt;
        vec_t<T,3,0212> const zyz, bgb, ptp;
        vec_t<T,3,0220> const zzx, bbr, pps;
        vec_t<T,3,0221> const zzy, bbg, ppt;
        vec_t<T,3,0222> const zzz, bbb, ppp;

        vec_t<T,4,00000> const xxxx, rrrr, ssss;
        vec_t<T,4,00001> const xxxy, rrrg, ssst;
        vec_t<T,4,00002> const xxxz, rrrb, sssp;
        vec_t<T,4,00010> const xxyx, rrgr, ssts;
        vec_t<T,4,00011> const xxyy, rrgg, sstt;
        vec_t<T,4,00012> const xxyz, rrgb, sstp;
        vec_t<T,4,00020> const xxzx, rrbr, ssps;
        vec_t<T,4,00021> const xxzy, rrbg, sspt;
        vec_t<T,4,00022> const xxzz, rrbb, sspp;
        vec_t<T,4,00100> const xyxx, rgrr, stss;
        vec_t<T,4,00101> const xyxy, rgrg, stst;
        vec_t<T,4,00102> const xyxz, rgrb, stsp;
        vec_t<T,4,00110> const xyyx, rggr, stts;
        vec_t<T,4,00111> const xyyy, rggg, sttt;
        vec_t<T,4,00112> const xyyz, rggb, sttp;
        vec_t<T,4,00120> const xyzx, rgbr, stps;
        vec_t<T,4,00121> const xyzy, rgbg, stpt;
        vec_t<T,4,00122> const xyzz, rgbb, stpp;
        vec_t<T,4,00200> const xzxx, rbrr, spss;
        vec_t<T,4,00201> const xzxy, rbrg, spst;
        vec_t<T,4,00202> const xzxz, rbrb, spsp;
        vec_t<T,4,00210> const xzyx, rbgr, spts;
        vec_t<T,4,00211> const xzyy, rbgg, sptt;
        vec_t<T,4,00212> const xzyz, rbgb, sptp;
        vec_t<T,4,00220> const xzzx, rbbr, spps;
        vec_t<T,4,00221> const xzzy, rbbg, sppt;
        vec_t<T,4,00222> const xzzz, rbbb, sppp;
        vec_t<T,4,01000> const yxxx, grrr, tsss;
        vec_t<T,4,01001> const yxxy, grrg, tsst;
        vec_t<T,4,01002> const yxxz, grrb, tssp;
        vec_t<T,4,01010> const yxyx, grgr, tsts;
        vec_t<T,4,01011> const yxyy, grgg, tstt;
        vec_t<T,4,01012> const yxyz, grgb, tstp;
        vec_t<T,4,01020> const yxzx, grbr, tsps;
        vec_t<T,4,01021> const yxzy, grbg, tspt;
        vec_t<T,4,01022> const yxzz, grbb, tspp;
        vec_t<T,4,01100> const yyxx, ggrr, ttss;
        vec_t<T,4,01101> const yyxy, ggrg, ttst;
        vec_t<T,4,01102> const yyxz, ggrb, ttsp;
        vec_t<T,4,01110> const yyyx, gggr, ttts;
        vec_t<T,4,01111> const yyyy, gggg, tttt;
        vec_t<T,4,01112> const yyyz, gggb, tttp;
        vec_t<T,4,01120> const yyzx, ggbr, ttps;
        vec_t<T,4,01121> const yyzy, ggbg, ttpt;
        vec_t<T,4,01122> const yyzz, ggbb, ttpp;
        vec_t<T,4,01200> const yzxx, gbrr, tpss;
        vec_t<T,4,01201> const yzxy, gbrg, tpst;
        vec_t<T,4,01202> const yzxz, gbrb, tpsp;
        vec_t<T,4,01210> const yzyx, gbgr, tpts;
        vec_t<T,4,01211> const yzyy, gbgg, tptt;
        vec_t<T,4,01212> const yzyz, gbgb, tptp;
        vec_t<T,4,01220> const yzzx, gbbr, tpps;
        vec_t<T,4,01221> const yzzy, gbbg, tppt;
        vec_t<T,4,01222> const yzzz, gbbb, tppp;
        vec_t<T,4,02000> const zxxx, brrr, psss;
        vec_t<T,4,02001> const zxxy, brrg, psst;
        vec_t<T,4,02002> const zxxz, brrb, pssp;
        vec_t<T,4,02010> const zxyx, brgr, psts;
        vec_t<T,4,02011> const zxyy, brgg, pstt;
        vec_t<T,4,02012> const zxyz, brgb, pstp;
        vec_t<T,4,02020> const zxzx, brbr, psps;
        vec_t<T,4,02021> const zxzy, brbg, pspt;
        vec_t<T,4,02022> const zxzz, brbb, pspp;
        vec_t<T,4,02100> const zyxx, bgrr, ptss;
        vec_t<T,4,02101> const zyxy, bgrg, ptst;
        vec_t<T,4,02102> const zyxz, bgrb, ptsp;
        vec_t<T,4,02110> const zyyx, bggr, ptts;
        vec_t<T,4,02111> const zyyy, bggg, pttt;
        vec_t<T,4,02112> const zyyz, bggb, pttp;
        vec_t<T,4,02120> const zyzx, bgbr, ptps;
        vec_t<T,4,02121> const zyzy, bgbg, ptpt;
        vec_t<T,4,02122> const zyzz, bgbb, ptpp;
        vec_t<T,4,02200> const zzxx, bbrr, ppss;
        vec_t<T,4,02201> const zzxy, bbrg, ppst;
        vec_t<T,4,02202> const zzxz, bbrb, ppsp;
        vec_t<T,4,02210> const zzyx, bbgr, ppts;
        vec_t<T,4,02211> const zzyy, bbgg, pptt;
        vec_t<T,4,02212> const zzyz, bbgb, pptp;
        vec_t<T,4,02220> const zzzx, bbbr, ppps;
        vec_t<T,4,02221> const zzzy, bbbg, pppt;
        vec_t<T,4,02222> const zzzz, bbbb, pppp;
#endif

        T m_data[count];
    };
};

/*
 * 4-element vectors
 */

template <typename T>
struct [[nodiscard]] vec_t<T,4>
  : public swizzle_ops::base<T>
{
    static int const count = 4;
    typedef T scalar_element;
    typedef T element;
    typedef vec_t<T,4> type;

    // Default constructor, copy constructor, and destructor
    inline constexpr vec_t() = default;
    inline constexpr vec_t(type const &v) = default;
    inline ~vec_t() = default;

    /* Implicit constructor for swizzling */
    template<int SWIZZLE>
    inline constexpr vec_t(vec_t<T, 4, SWIZZLE> const &v)
      : x(v[0]), y(v[1]), z(v[2]), w(v[3]) {}

    /* Explicit constructor for type conversion */
    template<typename U, int SWIZZLE>
    explicit inline constexpr vec_t(vec_t<U, 4, SWIZZLE> const &v)
      : x(T(v[0])), y(T(v[1])), z(T(v[2])), w(T(v[3])) {}

    /* Constructor for initializer_list. We need these ugly
     * loops until C++ lets us initialize m_data directly. */
    inline vec_t(std::initializer_list<element> const &list)
    {
        auto l = list.begin();
        for (int i = 0; i < count && l != list.end(); ++i, ++l)
            m_data[i] = *l;
        for (int i = (int)list.size(); i < count; ++i)
            m_data[i] = T(0);
    }

    /* Various explicit constructors */
    explicit inline constexpr vec_t(T X)
      : x(X), y(X), z(X), w(X) {}
    explicit inline constexpr vec_t(T X, T Y, T Z, T W)
      : x(X), y(Y), z(Z), w(W) {}
    explicit inline constexpr vec_t(vec_t<T,2> XY, T Z, T W)
      : x(XY.x), y(XY.y), z(Z), w(W) {}
    explicit inline constexpr vec_t(T X, vec_t<T,2> YZ, T W)
      : x(X), y(YZ.x), z(YZ.y), w(W) {}
    explicit inline constexpr vec_t(T X, T Y, vec_t<T,2> ZW)
      : x(X), y(Y), z(ZW.x), w(ZW.y) {}
    explicit inline constexpr vec_t(vec_t<T,2> XY, vec_t<T,2> ZW)
      : x(XY.x), y(XY.y), z(ZW.x), w(ZW.y) {}
    explicit inline constexpr vec_t(vec_t<T,3> XYZ, T W)
      : x(XYZ.x), y(XYZ.y), z(XYZ.z), w(W) {}
    explicit inline constexpr vec_t(T X, vec_t<T,3> YZW)
      : x(X), y(YZW.x), z(YZW.y), w(YZW.z) {}

    /* Factory for base axis vectors, e.g. [1,0,0,…,0] */
    static inline vec_t<T,4> axis(int i)
    {
        assert(i >= 0);
        assert(i < 4);
        return vec_t<T,4>(T(i == 0), T(i == 1), T(i == 2), T(i == 3));
    }

    LOL_COMMON_MEMBER_OPS(x)

    static const vec_t<T,4> zero;
    static const vec_t<T,4> axis_x;
    static const vec_t<T,4> axis_y;
    static const vec_t<T,4> axis_z;
    static const vec_t<T,4> axis_w;

    union
    {
        struct { T x, y, z, w; }; /* axis */
        struct { T r, g, b, a; }; /* red, green, blue, alpha */
        struct { T s, t, p, q; };

#if !_DOXYGEN_SKIP_ME
        vec_t<T,2,000> const xx, rr, ss;
        vec_t<T,2,001> const xy, rg, st;
        vec_t<T,2,002> const xz, rb, sp;
        vec_t<T,2,003> const xw, ra, sq;
        vec_t<T,2,010> const yx, gr, ts;
        vec_t<T,2,011> const yy, gg, tt;
        vec_t<T,2,012> const yz, gb, tp;
        vec_t<T,2,013> const yw, ga, tq;
        vec_t<T,2,020> const zx, br, ps;
        vec_t<T,2,021> const zy, bg, pt;
        vec_t<T,2,022> const zz, bb, pp;
        vec_t<T,2,023> const zw, ba, pq;
        vec_t<T,2,030> const wx, ar, qs;
        vec_t<T,2,031> const wy, ag, qt;
        vec_t<T,2,032> const wz, ab, qp;
        vec_t<T,2,033> const ww, aa, qq;

        vec_t<T,3,0000> const xxx, rrr, sss;
        vec_t<T,3,0001> const xxy, rrg, sst;
        vec_t<T,3,0002> const xxz, rrb, ssp;
        vec_t<T,3,0003> const xxw, rra, ssq;
        vec_t<T,3,0010> const xyx, rgr, sts;
        vec_t<T,3,0011> const xyy, rgg, stt;
        vec_t<T,3,0012> const xyz, rgb, stp;
        vec_t<T,3,0013> const xyw, rga, stq;
        vec_t<T,3,0020> const xzx, rbr, sps;
        vec_t<T,3,0021> const xzy, rbg, spt;
        vec_t<T,3,0022> const xzz, rbb, spp;
        vec_t<T,3,0023> const xzw, rba, spq;
        vec_t<T,3,0030> const xwx, rar, sqs;
        vec_t<T,3,0031> const xwy, rag, sqt;
        vec_t<T,3,0032> const xwz, rab, sqp;
        vec_t<T,3,0033> const xww, raa, sqq;
        vec_t<T,3,0100> const yxx, grr, tss;
        vec_t<T,3,0101> const yxy, grg, tst;
        vec_t<T,3,0102> const yxz, grb, tsp;
        vec_t<T,3,0103> const yxw, gra, tsq;
        vec_t<T,3,0110> const yyx, ggr, tts;
        vec_t<T,3,0111> const yyy, ggg, ttt;
        vec_t<T,3,0112> const yyz, ggb, ttp;
        vec_t<T,3,0113> const yyw, gga, ttq;
        vec_t<T,3,0120> const yzx, gbr, tps;
        vec_t<T,3,0121> const yzy, gbg, tpt;
        vec_t<T,3,0122> const yzz, gbb, tpp;
        vec_t<T,3,0123> const yzw, gba, tpq;
        vec_t<T,3,0130> const ywx, gar, tqs;
        vec_t<T,3,0131> const ywy, gag, tqt;
        vec_t<T,3,0132> const ywz, gab, tqp;
        vec_t<T,3,0133> const yww, gaa, tqq;
        vec_t<T,3,0200> const zxx, brr, pss;
        vec_t<T,3,0201> const zxy, brg, pst;
        vec_t<T,3,0202> const zxz, brb, psp;
        vec_t<T,3,0203> const zxw, bra, psq;
        vec_t<T,3,0210> const zyx, bgr, pts;
        vec_t<T,3,0211> const zyy, bgg, ptt;
        vec_t<T,3,0212> const zyz, bgb, ptp;
        vec_t<T,3,0213> const zyw, bga, ptq;
        vec_t<T,3,0220> const zzx, bbr, pps;
        vec_t<T,3,0221> const zzy, bbg, ppt;
        vec_t<T,3,0222> const zzz, bbb, ppp;
        vec_t<T,3,0223> const zzw, bba, ppq;
        vec_t<T,3,0230> const zwx, bar, pqs;
        vec_t<T,3,0231> const zwy, bag, pqt;
        vec_t<T,3,0232> const zwz, bab, pqp;
        vec_t<T,3,0233> const zww, baa, pqq;
        vec_t<T,3,0300> const wxx, arr, qss;
        vec_t<T,3,0301> const wxy, arg, qst;
        vec_t<T,3,0302> const wxz, arb, qsp;
        vec_t<T,3,0303> const wxw, ara, qsq;
        vec_t<T,3,0310> const wyx, agr, qts;
        vec_t<T,3,0311> const wyy, agg, qtt;
        vec_t<T,3,0312> const wyz, agb, qtp;
        vec_t<T,3,0313> const wyw, aga, qtq;
        vec_t<T,3,0320> const wzx, abr, qps;
        vec_t<T,3,0321> const wzy, abg, qpt;
        vec_t<T,3,0322> const wzz, abb, qpp;
        vec_t<T,3,0323> const wzw, aba, qpq;
        vec_t<T,3,0330> const wwx, aar, qqs;
        vec_t<T,3,0331> const wwy, aag, qqt;
        vec_t<T,3,0332> const wwz, aab, qqp;
        vec_t<T,3,0333> const www, aaa, qqq;

        vec_t<T,4,00000> const xxxx, rrrr, ssss;
        vec_t<T,4,00001> const xxxy, rrrg, ssst;
        vec_t<T,4,00002> const xxxz, rrrb, sssp;
        vec_t<T,4,00003> const xxxw, rrra, sssq;
        vec_t<T,4,00010> const xxyx, rrgr, ssts;
        vec_t<T,4,00011> const xxyy, rrgg, sstt;
        vec_t<T,4,00012> const xxyz, rrgb, sstp;
        vec_t<T,4,00013> const xxyw, rrga, sstq;
        vec_t<T,4,00020> const xxzx, rrbr, ssps;
        vec_t<T,4,00021> const xxzy, rrbg, sspt;
        vec_t<T,4,00022> const xxzz, rrbb, sspp;
        vec_t<T,4,00023> const xxzw, rrba, sspq;
        vec_t<T,4,00030> const xxwx, rrar, ssqs;
        vec_t<T,4,00031> const xxwy, rrag, ssqt;
        vec_t<T,4,00032> const xxwz, rrab, ssqp;
        vec_t<T,4,00033> const xxww, rraa, ssqq;
        vec_t<T,4,00100> const xyxx, rgrr, stss;
        vec_t<T,4,00101> const xyxy, rgrg, stst;
        vec_t<T,4,00102> const xyxz, rgrb, stsp;
        vec_t<T,4,00103> const xyxw, rgra, stsq;
        vec_t<T,4,00110> const xyyx, rggr, stts;
        vec_t<T,4,00111> const xyyy, rggg, sttt;
        vec_t<T,4,00112> const xyyz, rggb, sttp;
        vec_t<T,4,00113> const xyyw, rgga, sttq;
        vec_t<T,4,00120> const xyzx, rgbr, stps;
        vec_t<T,4,00121> const xyzy, rgbg, stpt;
        vec_t<T,4,00122> const xyzz, rgbb, stpp;
        vec_t<T,4,00123> const xyzw, rgba, stpq;
        vec_t<T,4,00130> const xywx, rgar, stqs;
        vec_t<T,4,00131> const xywy, rgag, stqt;
        vec_t<T,4,00132> const xywz, rgab, stqp;
        vec_t<T,4,00133> const xyww, rgaa, stqq;
        vec_t<T,4,00200> const xzxx, rbrr, spss;
        vec_t<T,4,00201> const xzxy, rbrg, spst;
        vec_t<T,4,00202> const xzxz, rbrb, spsp;
        vec_t<T,4,00203> const xzxw, rbra, spsq;
        vec_t<T,4,00210> const xzyx, rbgr, spts;
        vec_t<T,4,00211> const xzyy, rbgg, sptt;
        vec_t<T,4,00212> const xzyz, rbgb, sptp;
        vec_t<T,4,00213> const xzyw, rbga, sptq;
        vec_t<T,4,00220> const xzzx, rbbr, spps;
        vec_t<T,4,00221> const xzzy, rbbg, sppt;
        vec_t<T,4,00222> const xzzz, rbbb, sppp;
        vec_t<T,4,00223> const xzzw, rbba, sppq;
        vec_t<T,4,00230> const xzwx, rbar, spqs;
        vec_t<T,4,00231> const xzwy, rbag, spqt;
        vec_t<T,4,00232> const xzwz, rbab, spqp;
        vec_t<T,4,00233> const xzww, rbaa, spqq;
        vec_t<T,4,00300> const xwxx, rarr, sqss;
        vec_t<T,4,00301> const xwxy, rarg, sqst;
        vec_t<T,4,00302> const xwxz, rarb, sqsp;
        vec_t<T,4,00303> const xwxw, rara, sqsq;
        vec_t<T,4,00310> const xwyx, ragr, sqts;
        vec_t<T,4,00311> const xwyy, ragg, sqtt;
        vec_t<T,4,00312> const xwyz, ragb, sqtp;
        vec_t<T,4,00313> const xwyw, raga, sqtq;
        vec_t<T,4,00320> const xwzx, rabr, sqps;
        vec_t<T,4,00321> const xwzy, rabg, sqpt;
        vec_t<T,4,00322> const xwzz, rabb, sqpp;
        vec_t<T,4,00323> const xwzw, raba, sqpq;
        vec_t<T,4,00330> const xwwx, raar, sqqs;
        vec_t<T,4,00331> const xwwy, raag, sqqt;
        vec_t<T,4,00332> const xwwz, raab, sqqp;
        vec_t<T,4,00333> const xwww, raaa, sqqq;
        vec_t<T,4,01000> const yxxx, grrr, tsss;
        vec_t<T,4,01001> const yxxy, grrg, tsst;
        vec_t<T,4,01002> const yxxz, grrb, tssp;
        vec_t<T,4,01003> const yxxw, grra, tssq;
        vec_t<T,4,01010> const yxyx, grgr, tsts;
        vec_t<T,4,01011> const yxyy, grgg, tstt;
        vec_t<T,4,01012> const yxyz, grgb, tstp;
        vec_t<T,4,01013> const yxyw, grga, tstq;
        vec_t<T,4,01020> const yxzx, grbr, tsps;
        vec_t<T,4,01021> const yxzy, grbg, tspt;
        vec_t<T,4,01022> const yxzz, grbb, tspp;
        vec_t<T,4,01023> const yxzw, grba, tspq;
        vec_t<T,4,01030> const yxwx, grar, tsqs;
        vec_t<T,4,01031> const yxwy, grag, tsqt;
        vec_t<T,4,01032> const yxwz, grab, tsqp;
        vec_t<T,4,01033> const yxww, graa, tsqq;
        vec_t<T,4,01100> const yyxx, ggrr, ttss;
        vec_t<T,4,01101> const yyxy, ggrg, ttst;
        vec_t<T,4,01102> const yyxz, ggrb, ttsp;
        vec_t<T,4,01103> const yyxw, ggra, ttsq;
        vec_t<T,4,01110> const yyyx, gggr, ttts;
        vec_t<T,4,01111> const yyyy, gggg, tttt;
        vec_t<T,4,01112> const yyyz, gggb, tttp;
        vec_t<T,4,01113> const yyyw, ggga, tttq;
        vec_t<T,4,01120> const yyzx, ggbr, ttps;
        vec_t<T,4,01121> const yyzy, ggbg, ttpt;
        vec_t<T,4,01122> const yyzz, ggbb, ttpp;
        vec_t<T,4,01123> const yyzw, ggba, ttpq;
        vec_t<T,4,01130> const yywx, ggar, ttqs;
        vec_t<T,4,01131> const yywy, ggag, ttqt;
        vec_t<T,4,01132> const yywz, ggab, ttqp;
        vec_t<T,4,01133> const yyww, ggaa, ttqq;
        vec_t<T,4,01200> const yzxx, gbrr, tpss;
        vec_t<T,4,01201> const yzxy, gbrg, tpst;
        vec_t<T,4,01202> const yzxz, gbrb, tpsp;
        vec_t<T,4,01203> const yzxw, gbra, tpsq;
        vec_t<T,4,01210> const yzyx, gbgr, tpts;
        vec_t<T,4,01211> const yzyy, gbgg, tptt;
        vec_t<T,4,01212> const yzyz, gbgb, tptp;
        vec_t<T,4,01213> const yzyw, gbga, tptq;
        vec_t<T,4,01220> const yzzx, gbbr, tpps;
        vec_t<T,4,01221> const yzzy, gbbg, tppt;
        vec_t<T,4,01222> const yzzz, gbbb, tppp;
        vec_t<T,4,01223> const yzzw, gbba, tppq;
        vec_t<T,4,01230> const yzwx, gbar, tpqs;
        vec_t<T,4,01231> const yzwy, gbag, tpqt;
        vec_t<T,4,01232> const yzwz, gbab, tpqp;
        vec_t<T,4,01233> const yzww, gbaa, tpqq;
        vec_t<T,4,01300> const ywxx, garr, tqss;
        vec_t<T,4,01301> const ywxy, garg, tqst;
        vec_t<T,4,01302> const ywxz, garb, tqsp;
        vec_t<T,4,01303> const ywxw, gara, tqsq;
        vec_t<T,4,01310> const ywyx, gagr, tqts;
        vec_t<T,4,01311> const ywyy, gagg, tqtt;
        vec_t<T,4,01312> const ywyz, gagb, tqtp;
        vec_t<T,4,01313> const ywyw, gaga, tqtq;
        vec_t<T,4,01320> const ywzx, gabr, tqps;
        vec_t<T,4,01321> const ywzy, gabg, tqpt;
        vec_t<T,4,01322> const ywzz, gabb, tqpp;
        vec_t<T,4,01323> const ywzw, gaba, tqpq;
        vec_t<T,4,01330> const ywwx, gaar, tqqs;
        vec_t<T,4,01331> const ywwy, gaag, tqqt;
        vec_t<T,4,01332> const ywwz, gaab, tqqp;
        vec_t<T,4,01333> const ywww, gaaa, tqqq;
        vec_t<T,4,02000> const zxxx, brrr, psss;
        vec_t<T,4,02001> const zxxy, brrg, psst;
        vec_t<T,4,02002> const zxxz, brrb, pssp;
        vec_t<T,4,02003> const zxxw, brra, pssq;
        vec_t<T,4,02010> const zxyx, brgr, psts;
        vec_t<T,4,02011> const zxyy, brgg, pstt;
        vec_t<T,4,02012> const zxyz, brgb, pstp;
        vec_t<T,4,02013> const zxyw, brga, pstq;
        vec_t<T,4,02020> const zxzx, brbr, psps;
        vec_t<T,4,02021> const zxzy, brbg, pspt;
        vec_t<T,4,02022> const zxzz, brbb, pspp;
        vec_t<T,4,02023> const zxzw, brba, pspq;
        vec_t<T,4,02030> const zxwx, brar, psqs;
        vec_t<T,4,02031> const zxwy, brag, psqt;
        vec_t<T,4,02032> const zxwz, brab, psqp;
        vec_t<T,4,02033> const zxww, braa, psqq;
        vec_t<T,4,02100> const zyxx, bgrr, ptss;
        vec_t<T,4,02101> const zyxy, bgrg, ptst;
        vec_t<T,4,02102> const zyxz, bgrb, ptsp;
        vec_t<T,4,02103> const zyxw, bgra, ptsq;
        vec_t<T,4,02110> const zyyx, bggr, ptts;
        vec_t<T,4,02111> const zyyy, bggg, pttt;
        vec_t<T,4,02112> const zyyz, bggb, pttp;
        vec_t<T,4,02113> const zyyw, bgga, pttq;
        vec_t<T,4,02120> const zyzx, bgbr, ptps;
        vec_t<T,4,02121> const zyzy, bgbg, ptpt;
        vec_t<T,4,02122> const zyzz, bgbb, ptpp;
        vec_t<T,4,02123> const zyzw, bgba, ptpq;
        vec_t<T,4,02130> const zywx, bgar, ptqs;
        vec_t<T,4,02131> const zywy, bgag, ptqt;
        vec_t<T,4,02132> const zywz, bgab, ptqp;
        vec_t<T,4,02133> const zyww, bgaa, ptqq;
        vec_t<T,4,02200> const zzxx, bbrr, ppss;
        vec_t<T,4,02201> const zzxy, bbrg, ppst;
        vec_t<T,4,02202> const zzxz, bbrb, ppsp;
        vec_t<T,4,02203> const zzxw, bbra, ppsq;
        vec_t<T,4,02210> const zzyx, bbgr, ppts;
        vec_t<T,4,02211> const zzyy, bbgg, pptt;
        vec_t<T,4,02212> const zzyz, bbgb, pptp;
        vec_t<T,4,02213> const zzyw, bbga, pptq;
        vec_t<T,4,02220> const zzzx, bbbr, ppps;
        vec_t<T,4,02221> const zzzy, bbbg, pppt;
        vec_t<T,4,02222> const zzzz, bbbb, pppp;
        vec_t<T,4,02223> const zzzw, bbba, pppq;
        vec_t<T,4,02230> const zzwx, bbar, ppqs;
        vec_t<T,4,02231> const zzwy, bbag, ppqt;
        vec_t<T,4,02232> const zzwz, bbab, ppqp;
        vec_t<T,4,02233> const zzww, bbaa, ppqq;
        vec_t<T,4,02300> const zwxx, barr, pqss;
        vec_t<T,4,02301> const zwxy, barg, pqst;
        vec_t<T,4,02302> const zwxz, barb, pqsp;
        vec_t<T,4,02303> const zwxw, bara, pqsq;
        vec_t<T,4,02310> const zwyx, bagr, pqts;
        vec_t<T,4,02311> const zwyy, bagg, pqtt;
        vec_t<T,4,02312> const zwyz, bagb, pqtp;
        vec_t<T,4,02313> const zwyw, baga, pqtq;
        vec_t<T,4,02320> const zwzx, babr, pqps;
        vec_t<T,4,02321> const zwzy, babg, pqpt;
        vec_t<T,4,02322> const zwzz, babb, pqpp;
        vec_t<T,4,02323> const zwzw, baba, pqpq;
        vec_t<T,4,02330> const zwwx, baar, pqqs;
        vec_t<T,4,02331> const zwwy, baag, pqqt;
        vec_t<T,4,02332> const zwwz, baab, pqqp;
        vec_t<T,4,02333> const zwww, baaa, pqqq;
        vec_t<T,4,03000> const wxxx, arrr, qsss;
        vec_t<T,4,03001> const wxxy, arrg, qsst;
        vec_t<T,4,03002> const wxxz, arrb, qssp;
        vec_t<T,4,03003> const wxxw, arra, qssq;
        vec_t<T,4,03010> const wxyx, argr, qsts;
        vec_t<T,4,03011> const wxyy, argg, qstt;
        vec_t<T,4,03012> const wxyz, argb, qstp;
        vec_t<T,4,03013> const wxyw, arga, qstq;
        vec_t<T,4,03020> const wxzx, arbr, qsps;
        vec_t<T,4,03021> const wxzy, arbg, qspt;
        vec_t<T,4,03022> const wxzz, arbb, qspp;
        vec_t<T,4,03023> const wxzw, arba, qspq;
        vec_t<T,4,03030> const wxwx, arar, qsqs;
        vec_t<T,4,03031> const wxwy, arag, qsqt;
        vec_t<T,4,03032> const wxwz, arab, qsqp;
        vec_t<T,4,03033> const wxww, araa, qsqq;
        vec_t<T,4,03100> const wyxx, agrr, qtss;
        vec_t<T,4,03101> const wyxy, agrg, qtst;
        vec_t<T,4,03102> const wyxz, agrb, qtsp;
        vec_t<T,4,03103> const wyxw, agra, qtsq;
        vec_t<T,4,03110> const wyyx, aggr, qtts;
        vec_t<T,4,03111> const wyyy, aggg, qttt;
        vec_t<T,4,03112> const wyyz, aggb, qttp;
        vec_t<T,4,03113> const wyyw, agga, qttq;
        vec_t<T,4,03120> const wyzx, agbr, qtps;
        vec_t<T,4,03121> const wyzy, agbg, qtpt;
        vec_t<T,4,03122> const wyzz, agbb, qtpp;
        vec_t<T,4,03123> const wyzw, agba, qtpq;
        vec_t<T,4,03130> const wywx, agar, qtqs;
        vec_t<T,4,03131> const wywy, agag, qtqt;
        vec_t<T,4,03132> const wywz, agab, qtqp;
        vec_t<T,4,03133> const wyww, agaa, qtqq;
        vec_t<T,4,03200> const wzxx, abrr, qpss;
        vec_t<T,4,03201> const wzxy, abrg, qpst;
        vec_t<T,4,03202> const wzxz, abrb, qpsp;
        vec_t<T,4,03203> const wzxw, abra, qpsq;
        vec_t<T,4,03210> const wzyx, abgr, qpts;
        vec_t<T,4,03211> const wzyy, abgg, qptt;
        vec_t<T,4,03212> const wzyz, abgb, qptp;
        vec_t<T,4,03213> const wzyw, abga, qptq;
        vec_t<T,4,03220> const wzzx, abbr, qpps;
        vec_t<T,4,03221> const wzzy, abbg, qppt;
        vec_t<T,4,03222> const wzzz, abbb, qppp;
        vec_t<T,4,03223> const wzzw, abba, qppq;
        vec_t<T,4,03230> const wzwx, abar, qpqs;
        vec_t<T,4,03231> const wzwy, abag, qpqt;
        vec_t<T,4,03232> const wzwz, abab, qpqp;
        vec_t<T,4,03233> const wzww, abaa, qpqq;
        vec_t<T,4,03300> const wwxx, aarr, qqss;
        vec_t<T,4,03301> const wwxy, aarg, qqst;
        vec_t<T,4,03302> const wwxz, aarb, qqsp;
        vec_t<T,4,03303> const wwxw, aara, qqsq;
        vec_t<T,4,03310> const wwyx, aagr, qqts;
        vec_t<T,4,03311> const wwyy, aagg, qqtt;
        vec_t<T,4,03312> const wwyz, aagb, qqtp;
        vec_t<T,4,03313> const wwyw, aaga, qqtq;
        vec_t<T,4,03320> const wwzx, aabr, qqps;
        vec_t<T,4,03321> const wwzy, aabg, qqpt;
        vec_t<T,4,03322> const wwzz, aabb, qqpp;
        vec_t<T,4,03323> const wwzw, aaba, qqpq;
        vec_t<T,4,03330> const wwwx, aaar, qqqs;
        vec_t<T,4,03331> const wwwy, aaag, qqqt;
        vec_t<T,4,03332> const wwwz, aaab, qqqp;
        vec_t<T,4,03333> const wwww, aaaa, qqqq;
#endif

        T m_data[count];
    };
};

/*
 * stdstream method implementation
 */

template<typename U, int N, int SWIZZLE = FULL_SWIZZLE>
std::ostream &operator<<(std::ostream &stream, vec_t<U,N> const &v)
{
    stream << '(';
    for (int i = 0; i < N; ++i)
        stream << v[i] << (i == N - 1 ? ")" : ", ");
    return stream;
}

/*
 * vec_t *(scalar, vec_t)
 */

template<typename T, int N, int SWIZZLE>
static inline typename std::enable_if<SWIZZLE != FULL_SWIZZLE, vec_t<T,N>>::type
operator *(T const &val, vec_t<T,N,SWIZZLE> const &a)
{
    vec_t<T,N> ret;
    for (int i = 0; i < N; ++i)
        ret[i] = val * a[i];
    return ret;
}

/*
 * vec_t min|max|fmod(vec_t, vec_t|scalar)
 * vec_t min|max|fmod(scalar, vec_t)
 */

#define LOL_SWIZZLE_V_VV_FUN(fun) \
    template<typename T, int N, int SWIZZLE1, int SWIZZLE2> \
    inline vec_t<T,N> fun(vec_t<T,N,SWIZZLE1> const &a, \
                          vec_t<T,N,SWIZZLE2> const &b) \
    { \
        using std::fun; \
        vec_t<T,N> ret; \
        for (int i = 0; i < N; ++i) \
            ret[i] = fun(a[i], b[i]); \
        return ret; \
    } \
    \
    template<typename T, int N, int SWIZZLE> \
    inline vec_t<T,N> fun(vec_t<T,N,SWIZZLE> const &a, T const &b) \
    { \
        using std::fun; \
        vec_t<T,N> ret; \
        for (int i = 0; i < N; ++i) \
            ret[i] = fun(a[i], b); \
        return ret; \
    } \
    \
    template<typename T, int N, int SWIZZLE> \
    inline vec_t<T,N> fun(T const &a, vec_t<T,N,SWIZZLE> const &b) \
    { \
        using std::fun; \
        vec_t<T,N> ret; \
        for (int i = 0; i < N; ++i) \
            ret[i] = fun(a, b[i]); \
        return ret; \
    }

LOL_SWIZZLE_V_VV_FUN(min)
LOL_SWIZZLE_V_VV_FUN(max)
LOL_SWIZZLE_V_VV_FUN(fmod)

#undef LOL_SWIZZLE_V_VV_FUN

/*
 * vec_t clamp(vec_t, vec_t, vec_t)
 * vec_t clamp(vec_t, scalar, vec_t)
 * vec_t clamp(vec_t, vec_t, scalar)
 * vec_t clamp(vec_t, scalar, scalar)
 */

template<typename T, int N, int SWIZZLE1, int SWIZZLE2, int SWIZZLE3>
static inline vec_t<T,N> clamp(vec_t<T,N,SWIZZLE1> const &x,
                               vec_t<T,N,SWIZZLE2> const &a,
                               vec_t<T,N,SWIZZLE3> const &b)
{
    return max(min(x, b), a);
}

template<typename T, int N, int SWIZZLE1, int SWIZZLE2>
static inline vec_t<T,N> clamp(vec_t<T,N,SWIZZLE1> const &x,
                               T const &a,
                               vec_t<T,N,SWIZZLE2> const &b)
{
    return max(min(x, b), a);
}

template<typename T, int N, int SWIZZLE1, int SWIZZLE2>
static inline vec_t<T,N> clamp(vec_t<T,N,SWIZZLE1> const &x,
                               vec_t<T,N,SWIZZLE2> const &a,
                               T const &b)
{
    return max(min(x, b), a);
}

template<typename T, int N, int SWIZZLE1>
static inline vec_t<T,N> clamp(vec_t<T,N,SWIZZLE1> const &x,
                               T const &a,
                               T const &b)
{
    return max(min(x, b), a);
}

/*
 * vec_t mix(vec_t, vec_t, vec_t)
 * vec_t mix(vec_t, vec_t, scalar)
 */

template<typename T, int N, int SWIZZLE1, int SWIZZLE2, int SWIZZLE3>
static inline vec_t<T,N> mix(vec_t<T,N,SWIZZLE1> const &x,
                             vec_t<T,N,SWIZZLE2> const &y,
                             vec_t<T,N,SWIZZLE3> const &a)
{
    return x + a * (y - x);
}

template<typename T, int N, int SWIZZLE1, int SWIZZLE2>
static inline vec_t<T,N> mix(vec_t<T,N,SWIZZLE1> const &x,
                             vec_t<T,N,SWIZZLE2> const &y,
                             T const &a)
{
    return x + a * (y - x);
}

/*
 * Some GLSL-like functions.
 */

template<typename T, int N, int SWIZZLE1, int SWIZZLE2> [[nodiscard]]
static inline T dot(vec_t<T,N,SWIZZLE1> const &a,
                    vec_t<T,N,SWIZZLE2> const &b)
{
    T ret(0);
    for (int i = 0; i < N; ++i)
        ret += a[i] * b[i];
    return ret;
}

template<typename T, int N, int SWIZZLE> [[nodiscard]]
static inline T sqlength(vec_t<T,N,SWIZZLE> const &a)
{
    return dot(a, a);
}

template<typename T, int N, int SWIZZLE> [[nodiscard]]
static inline T length(vec_t<T,N,SWIZZLE> const &a)
{
    using std::sqrt;
    /* FIXME: this is not very nice */
    return T(sqrt(sqlength(a)));
}

template<typename T, int N, int SWIZZLE1, int SWIZZLE2>
static inline vec_t<T,N> lerp(vec_t<T,N,SWIZZLE1> const &a,
                              vec_t<T,N,SWIZZLE2> const &b,
                              T const &s)
{
    vec_t<T,N> ret;
    for (int i = 0; i < N; ++i)
        ret[i] = a[i] + s * (b[i] - a[i]);
    return ret;
}

template<typename T, int N, int SWIZZLE1, int SWIZZLE2> [[nodiscard]]
static inline T distance(vec_t<T,N,SWIZZLE1> const &a,
                         vec_t<T,N,SWIZZLE2> const &b)
{
    return length(a - b);
}

template<typename T, int N, int SWIZZLE>
static inline vec_t<T,N> fract(vec_t<T,N,SWIZZLE> const &a)
{
    vec_t<T,N> ret;
    for (int i = 0; i < N; ++i)
        ret[i] = fract(a[i]);
    return ret;
}

template<typename T, int N, int SWIZZLE>
static inline vec_t<T,N> saturate(vec_t<T,N,SWIZZLE> const &a)
{
    vec_t<T,N> ret;
    for (int i = 0; i < N; ++i)
        ret[i] = saturate(a[i]);
    return ret;
}

template<typename T, int N, int SWIZZLE>
static inline vec_t<T,N> normalize(vec_t<T,N,SWIZZLE> const &a)
{
    T norm = T(length(a));
    return norm ? a / norm : vec_t<T,N>(T(0));
}

// We define fabs() because that’s the C++ std library uses for
// floating-point numbers, and abs() because GLSL does.
template<typename T, int N, int SWIZZLE>
static inline vec_t<T,N> fabs(vec_t<T,N,SWIZZLE> const &a)
{
    using std::fabs;
    vec_t<T,N> ret;
    for (int i = 0; i < N; ++i)
        ret[i] = fabs(a[i]);
    return ret;
}

template<typename T, int N, int SWIZZLE>
static inline vec_t<T,N> abs(vec_t<T,N,SWIZZLE> const &a)
{
    return fabs(a);
}

template<typename T, int N, int SWIZZLE>
static inline vec_t<T,N> degrees(vec_t<T,N,SWIZZLE> const &a)
{
    vec_t<T,N> ret;
    for (int i = 0; i < N; ++i)
        ret[i] = degrees(a[i]);
    return ret;
}

template<typename T, int N, int SWIZZLE>
static inline vec_t<T, N> radians(vec_t<T, N, SWIZZLE> const &a)
{
    vec_t<T, N> ret;
    for (int i = 0; i < N; ++i)
        ret[i] = radians(a[i]);
    return ret;
}

// Cartesian operation consider that you're in spherical coordinate
template<typename T, int SWIZZLE>
static inline vec_t<T, 2> cartesian(vec_t<T, 2, SWIZZLE> const &a)
{
    using std::sin, std::cos;

    vec_t<T, 2> ret;
    ret.x = a[0] * cos(a[1]);
    ret.y = a[0] * sin(a[1]);
    return ret;
}

template<typename T, int SWIZZLE>
static inline vec_t<T, 3> cartesian(vec_t<T, 3, SWIZZLE> const &a)
{
    using std::sin, std::cos;

    vec_t<T, 3> ret;
    ret.x = a[0] * sin(a[2]) * cos(a[1]);
    ret.y = a[0] * cos(a[2]);
    ret.z = a[0] * sin(a[2]) * sin(a[1]);
    return ret;
}

// Spherical operation consider that you're in cartesian coordinate
template<typename T, int SWIZZLE>
static inline vec_t<T, 2> spherical(vec_t<T, 2, SWIZZLE> const &a)
{
    vec_t<T, 2> ret;
    ret[0] = sqlength(a);
    ret[1] = atan2(a.y, a.x);
    return ret;
}

template<typename T, int SWIZZLE>
static inline vec_t<T, 3> spherical(vec_t<T, 3, SWIZZLE> const &a)
{
    using std::atan, std::acos;

    vec_t<T, 3> ret;
    ret[0] = sqlength(a);
    ret[1] = atan(a.y / a.x);
    ret[2] = acos(a.z / ret[0]);
    return ret;
}

/*
 * C++11 iterators
 */

template<typename T, int N, int SWIZZLE>
class vec_const_iter
{
public:
    inline vec_const_iter(vec_t<T,N,SWIZZLE> const &vec, int pos)
      : m_vec(vec),
        m_pos(pos)
    { }

    inline bool operator !=(vec_const_iter<T,N,SWIZZLE> const & that) const
    {
        return m_pos != that.m_pos;
    }

    template<int S = SWIZZLE>
    inline typename std::enable_if<S != FULL_SWIZZLE, T const &>::type
    operator *() const
    {
        return m_vec[m_pos];
    }

    template<int S = SWIZZLE>
    inline typename std::enable_if<S == FULL_SWIZZLE, T const &>::type
    operator *() const
    {
        return m_vec[m_pos];
    }

    inline vec_const_iter<T,N,SWIZZLE> & operator ++()
    {
        ++m_pos;
        return *this;
    }

private:
    vec_t<T,N,SWIZZLE> const &m_vec;
    int m_pos;
};

/* FIXME: do we need a non-const iterator? Looks almost useless to me,
 * and a lot of hassle with swizzling. */
template<typename T, int N, int SWIZZLE>
inline vec_const_iter<T,N,SWIZZLE> begin(vec_t<T,N,SWIZZLE> const &a)
{
    return vec_const_iter<T,N,SWIZZLE>(a, 0);
}

template<typename T, int N, int SWIZZLE>
inline vec_const_iter<T,N,SWIZZLE> end(vec_t<T,N,SWIZZLE> const &a)
{
    return vec_const_iter<T,N,SWIZZLE>(a, N);
}

/*
 * Constants
 */

template<typename T, int N>
vec_t<T,N> const vec_t<T,N>::zero = vec_t<T,N>(T(0));
template<typename T>
vec_t<T,2> const vec_t<T,2>::zero = vec_t<T,2>(T(0));
template<typename T>
vec_t<T,3> const vec_t<T,3>::zero = vec_t<T,3>(T(0));
template<typename T>
vec_t<T,4> const vec_t<T,4>::zero = vec_t<T,4>(T(0));

template<typename T>
vec_t<T,2> const vec_t<T,2>::axis_x = vec_t<T,2>(T(1), T(0));
template<typename T>
vec_t<T,2> const vec_t<T,2>::axis_y = vec_t<T,2>(T(0), T(1));

template<typename T>
vec_t<T,3> const vec_t<T,3>::axis_x = vec_t<T,3>(T(1), T(0), T(0));
template<typename T>
vec_t<T,3> const vec_t<T,3>::axis_y = vec_t<T,3>(T(0), T(1), T(0));
template<typename T>
vec_t<T,3> const vec_t<T,3>::axis_z = vec_t<T,3>(T(0), T(0), T(1));

template<typename T>
vec_t<T,4> const vec_t<T,4>::axis_x = vec_t<T,4>(T(1), T(0), T(0), T(0));
template<typename T>
vec_t<T,4> const vec_t<T,4>::axis_y = vec_t<T,4>(T(0), T(1), T(0), T(0));
template<typename T>
vec_t<T,4> const vec_t<T,4>::axis_z = vec_t<T,4>(T(0), T(0), T(1), T(0));
template<typename T>
vec_t<T,4> const vec_t<T,4>::axis_w = vec_t<T,4>(T(0), T(0), T(0), T(1));

//
// Generic GLSL-like type names
//

#define T_(tleft, tright, suffix) \
    typedef tleft half tright f16##suffix; \
    typedef tleft float tright suffix; \
    typedef tleft double tright d##suffix; \
    typedef tleft ldouble tright f128##suffix; \
    typedef tleft int8_t tright i8##suffix; \
    typedef tleft uint8_t tright u8##suffix; \
    typedef tleft int16_t tright i16##suffix; \
    typedef tleft uint16_t tright u16##suffix; \
    typedef tleft int32_t tright i##suffix; \
    typedef tleft uint32_t tright u##suffix; \
    typedef tleft int64_t tright i64##suffix; \
    typedef tleft uint64_t tright u64##suffix; \
    typedef tleft real tright r##suffix;

// Idiotic hack to put "," inside a macro argument
#define C_ ,

T_(vec_t<, C_ 2>, vec2)
T_(vec_t<, C_ 3>, vec3)
T_(vec_t<, C_ 4>, vec4)
T_(vec_t<, C_ 5>, vec5)
T_(vec_t<, C_ 6>, vec6)
T_(vec_t<, C_ 7>, vec7)
T_(vec_t<, C_ 8>, vec8)
T_(vec_t<, C_ 9>, vec9)
T_(vec_t<, C_ 10>, vec10)
T_(vec_t<, C_ 11>, vec11)
T_(vec_t<, C_ 12>, vec12)

#undef C_
#undef T_

static_assert(sizeof(i8vec2) == 2, "sizeof(i8vec2) == 2");
static_assert(sizeof(i16vec2) == 4, "sizeof(i16vec2) == 4");
static_assert(sizeof(ivec2) == 8, "sizeof(ivec2) == 8");
static_assert(sizeof(i64vec2) == 16, "sizeof(i64vec2) == 16");

static_assert(sizeof(vec2) == 8, "sizeof(vec2) == 8");
static_assert(sizeof(dvec2) == 16, "sizeof(dvec2) == 16");

static_assert(sizeof(i8vec3) == 3, "sizeof(i8vec3) == 3");
static_assert(sizeof(i16vec3) == 6, "sizeof(i16vec3) == 6");
static_assert(sizeof(ivec3) == 12, "sizeof(ivec3) == 12");
static_assert(sizeof(i64vec3) == 24, "sizeof(i64vec3) == 24");

static_assert(sizeof(vec3) == 12, "sizeof(vec3) == 12");
static_assert(sizeof(dvec3) == 24, "sizeof(dvec3) == 24");

static_assert(sizeof(i8vec4) == 4, "sizeof(i8vec4) == 4");
static_assert(sizeof(i16vec4) == 8, "sizeof(i16vec4) == 8");
static_assert(sizeof(ivec4) == 16, "sizeof(ivec4) == 16");
static_assert(sizeof(i64vec4) == 32, "sizeof(i64vec4) == 32");

static_assert(sizeof(vec4) == 16, "sizeof(vec4) == 16");
static_assert(sizeof(dvec4) == 32, "sizeof(dvec4) == 32");

//
// HLSL/Cg-compliant type names
//

typedef vec2 float2;
typedef vec3 float3;
typedef vec4 float4;
typedef vec5 float5;
typedef vec6 float6;
typedef vec7 float7;
typedef vec8 float8;
typedef vec9 float9;
typedef vec10 float10;
typedef vec11 float11;
typedef vec12 float12;

typedef f16vec2 half2;
typedef f16vec3 half3;
typedef f16vec4 half4;

typedef ivec2 int2;
typedef ivec3 int3;
typedef ivec4 int4;
typedef ivec5 int5;
typedef ivec6 int6;
typedef ivec7 int7;
typedef ivec8 int8;
typedef ivec9 int9;
typedef ivec10 int10;
typedef ivec11 int11;
typedef ivec12 int12;

} // namespace lol

#include "vector.ipp"
