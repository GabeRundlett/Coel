#pragma once
#include <array>
#include <cstdint>

namespace detail {
    template <typename value_t, size_t n> struct vec_members_impl {
        std::array<value_t, n> array;
    };
    template <typename value_t> struct vec_members_impl<value_t, 2> {
        union {
            struct {
                value_t x, y;
            };
            std::array<value_t, 2> array;
        };
    };
    template <typename value_t> struct vec_members_impl<value_t, 3> {
        union {
            struct {
                value_t x, y, z;
            };
            std::array<value_t, 3> array;
        };
    };
    template <typename value_t> struct vec_members_impl<value_t, 4> {
        union {
            struct {
                value_t x, y, z, w;
            };
            std::array<value_t, 4> array;
        };
    };
    template <typename value_t, size_t n>
    struct vec_members : vec_members_impl<value_t, n> {
        constexpr value_t & operator[](size_t i) noexcept {
            return vec_members_impl<value_t, n>::array[i];
        }
        constexpr const value_t & operator[](size_t i) const noexcept {
            return vec_members_impl<value_t, n>::array[i];
        }
    };
} // namespace detail

template <typename value_t, size_t n> struct vec : detail::vec_members<value_t, n> {
    using vec_type = vec<value_t, n>;

    constexpr auto operator+(const vec_type & other) const {
        vec_type result;
        for (int i = 0; i < n; ++i) result[i] = (*this)[i] + other[i];
        return result;
    }
    constexpr auto operator-(const vec_type & other) const {
        vec_type result;
        for (int i = 0; i < n; ++i) result[i] = (*this)[i] - other[i];
        return result;
    }
    constexpr auto operator*(const vec_type & other) const {
        vec_type result;
        for (int i = 0; i < n; ++i) result[i] = (*this)[i] * other[i];
        return result;
    }
    constexpr auto operator*(const value_t & other) const {
        vec_type result;
        for (int i = 0; i < n; ++i) result[i] = (*this)[i] * other;
        return result;
    }
    constexpr auto operator/(const value_t & other) const {
        vec_type result;
        for (int i = 0; i < n; ++i) result[i] = (*this)[i] / other;
        return result;
    }
    constexpr auto & operator+=(const vec_type & other) {
        return (*this = *this + other);
    }
    constexpr auto & operator-=(const vec_type & other) {
        return (*this = *this - other);
    }
    constexpr auto & operator*=(const value_t & other) { return (*this = *this * other); }
    constexpr auto & operator/=(const value_t & other) { return (*this = *this / other); }
    constexpr auto   dot(const vec_type & other) const {
        value_t result = 0;
        for (int i = 0; i < n; ++i) result += (*this)[i] * other[i];
        return result;
    }
};

template <typename value_t, size_t n> inline auto length(const vec<value_t, n> & v) {
    return sqrt(v.dot(v));
}

template <typename value_t, size_t n> inline auto normalize(const vec<value_t, n> & v) {
    return v / length(v);
}
