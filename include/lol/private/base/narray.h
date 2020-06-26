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
// An n-dimensional row-major array template class
//

#include <lol/vector>  // lol::vec_t
#include <vector>      // std::vector
#include <utility>     // std::index_sequence
#include <type_traits> // std::remove_const

namespace lol
{

template<typename T, size_t N> class narray_span;

//
// Common base class for narray and narray_span
//

template<typename T, size_t N, template<typename, size_t> typename container_type>
class [[nodiscard]] narray_base
{
public:
    using value_type = T;

    // Return sizes of each dimension
    inline vec_t<int, N> sizes() const
    {
        return vec_t<int, N>(m_sizes);
    }

    // Size in number of elements and in bytes
    inline size_t size() const { return size_helper(std::make_index_sequence<N>{}); }
    inline size_t bytes() const { return size() * sizeof(value_type); }

    // Access element i in row-major fashion
    inline value_type &operator[](size_t i)
    {
        return data()[i];
    }

    inline value_type const &operator[](size_t i) const
    {
        return data()[i];
    }

    // Access element (i, j, .., z)
    template<typename... I>
    inline value_type &operator()(I... indices)
    {
        static_assert(N == sizeof...(I));
        return data()[offset(indices...)];
    }

    template<typename... I>
    inline value_type const &operator()(I... indices) const
    {
        static_assert(N == sizeof...(I));
        return data()[offset(indices...)];
    }

    inline value_type &operator()(vec_t<int, N> const &indices)
    {
        return data()[offset_helper(indices, std::make_index_sequence<N>{})];
    }

    inline value_type const &operator()(vec_t<int, N> const &indices) const
    {
        return data()[offset_helper(indices, std::make_index_sequence<N>{})];
    }

    // Use CRTP to access data() from the child class
    inline value_type *data()
    {
        return reinterpret_cast<container_type<T, N> &>(*this).data();
    }

    inline value_type const *data() const
    {
        return reinterpret_cast<container_type<T, N> const &>(*this).data();
    }

    // Create span (similar to std::span) with some const correctness
    inline narray_span<T, N> span()
    {
        return narray_span<T, N>(*this);
    }

    inline narray_span<T const, N> span() const
    {
        return narray_span<T const, N>(*this);
    }

protected:
    template <size_t... I>
    inline size_t size_helper(std::index_sequence<I...>) const
    {
        return (size_t(m_sizes[I]) * ... * 1);
    }

    template<typename... I>
    inline size_t offset(size_t i, I... indices) const
    {
        if constexpr(sizeof...(I) > 0)
            i += m_sizes[N - sizeof...(I) - 1] * offset(indices...);
        return i;
    }

    template <size_t... I>
    inline size_t offset_helper(vec_t<int, N> const &sizes, std::index_sequence<I...>) const
    {
        return offset(size_t(sizes[I])...);
    }

    vec_t<size_t, N> m_sizes { 0 };
};


//
// C++11 iterators
//

template<typename T, size_t N, template<typename, size_t> typename U>
T *begin(narray_base<T, N, U> &a) { return a.data(); }

template<typename T, size_t N, template<typename, size_t> typename U>
T *end(narray_base<T, N, U> &a) { return a.data() + a.size(); }

template<typename T, size_t N, template<typename, size_t> typename U>
T const *begin(narray_base<T, N, U> const &a) { return a.data(); }

template<typename T, size_t N, template<typename, size_t> typename U>
T const *end(narray_base<T, N, U> const &a) { return a.data() + a.size(); }

//
// N-dimensional array
//

template<typename T, size_t N>
class [[nodiscard]] narray : public narray_base<T, N, narray>
{
public:
    using value_type = T;

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

    // Resize array with a list of sizes (size1, size2, ..., sizen)
    template<typename... I>
    inline void resize(I... sizes)
    {
        static_assert(N == sizeof...(I));
        this->m_sizes = { size_t(sizes)... };
        m_data.resize(this->size());
    }

    template<typename U>
    inline void resize(vec_t<U, N> const &sizes)
    {
        this->m_sizes = vec_t<size_t, N>(sizes);
        m_data.resize(this->size());
    }

    // Access data directly
    inline value_type *data() { return m_data.data(); }
    inline value_type const *data() const { return m_data.data(); }

private:
    std::vector<T> m_data;
};

template<typename T> using array2d = narray<T, 2>;
template<typename T> using array3d = narray<T, 3>;

//
// N-dimensional array span
//

template<typename T, size_t N>
class [[nodiscard]] narray_span : public narray_base<T, N, narray_span>
{
public:
    using value_type = T;

    template<template<typename, size_t> typename U>
    inline narray_span(narray_base<T, N, U> &other)
      : m_data(other.data())
    {
        this->m_sizes = vec_t<size_t, N>(other.sizes());
    }

    // Create an narray_span<T const> from a const narray_base<T>
    template<template<typename, size_t> typename U, bool V = std::is_const<T>::value>
    inline narray_span(narray_base<typename std::remove_const<T>::type, N, U> const &other)
      : m_data(other.data())
    {
        this->m_sizes = vec_t<size_t, N>(other.sizes());
    }

    // Access data directly
    inline value_type *data() { return m_data; }
    inline value_type const *data() const { return m_data; }

private:
    T *m_data;
};

template<typename T> using span2d = narray_span<T, 2>;
template<typename T> using span3d = narray_span<T, 3>;

} // namespace lol

