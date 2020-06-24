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
// The narray class
// ————————————————
// An N-dimensional row-major array template class
//

#include <lol/vector>  // lol::vec_t
#include <vector>      // std::vector

namespace lol
{

template<typename T, size_t N>
class [[nodiscard]] narray
{
public:
    typedef T value_type;

    inline narray() = default;
    inline ~narray() = default;

    // Construct array with args: size1, size2, ..., sizen
    template<typename... I> inline narray(I... sizes)
    {
        static_assert(N == sizeof...(I));
        resize(sizes...);
    }

    // Construct array with integer vec_t arg:
    inline narray(vec_t<int, N> const &sizes)
    {
        resize(sizes);
    }

    // Empty array
    inline void clear() { resize(vec_t<size_t, N>(0)); }

    // Access element i
    inline value_type &operator[](size_t i)
    {
        return m_data[i];
    }

    inline value_type const &operator[](size_t i) const
    {
        return m_data[i];
    }

    // Access element (i, j, .., z) in row-major fashion
    template<typename... I>
    inline value_type &operator()(I... indices)
    {
        static_assert(N == sizeof...(I));
        return m_data[offset(indices...)];
    }

    template<typename... I>
    inline value_type const &operator()(I... indices) const
    {
        static_assert(N == sizeof...(I));
        return m_data[offset(indices...)];
    }

    inline value_type &operator()(vec_t<int, N> const &indices)
    {
        return m_data[offset_helper(indices, std::make_index_sequence<N>{})];
    }

    inline value_type const &operator()(vec_t<int, N> const &indices) const
    {
        return m_data[offset_helper(indices, std::make_index_sequence<N>{})];
    }

    // Resize array with a list of sizes (size1, size2, ..., sizen)
    template<typename... I>
    inline void resize(I... sizes)
    {
        static_assert(N == sizeof...(I));
        m_data.resize((size_t(sizes) * ... * 1));
        m_sizes = { size_t(sizes)... };
    }

    inline void resize(vec_t<int, N> const &sizes)
    {
        resize_helper(sizes, std::make_index_sequence<N>{});
    }

    // Return sizes of each dimension
    inline vec_t<int, N> sizes() const
    {
        return vec_t<int, N>(this->m_sizes);
    }

    inline value_type *data() { return m_data.data(); }
    inline value_type const *data() const { return m_data.data(); }
    inline size_t size() const { return m_data.size(); }
    inline size_t bytes() const { return size() * sizeof(value_type); }

private:
    template<typename... I> size_t offset(size_t i, I... indices) const
    {
        if constexpr(sizeof...(I) > 0)
            i += m_sizes[N - sizeof...(I) - 1] * offset(indices...);
        return i;
    }

    template <std::size_t... I>
    size_t offset_helper(vec_t<int, N> const &sizes, std::index_sequence<I...>) const
    {
        return offset(size_t(sizes[I])...);
    }

    template <std::size_t... I>
    void resize_helper(vec_t<int, N> const &sizes, std::index_sequence<I...>)
    {
        resize(sizes[I]...);
    }

    vec_t<size_t, N> m_sizes { 0 };
    std::vector<T> m_data;
};

template<typename T> using array2d = narray<T, 2>;
template<typename T> using array3d = narray<T, 3>;

} // namespace lol

