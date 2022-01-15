#pragma once
#include "vector.hpp"
#include <cmath>

template <typename value_t, size_t m, size_t n> struct mat : vec<vec<value_t, n>, m> {
    using mat_type                                = mat<value_t, m, n>;
    template <size_t p> using mul_mat_other_type  = mat<value_t, n, p>;
    template <size_t p> using mul_mat_result_type = mat<value_t, m, p>;

    template <size_t p> constexpr auto operator*(const mat<value_t, n, p> & other) {
        auto   c = mat<value_t, m, p>{};
        auto & a = *this;
        auto & b = other;
        for (int i = 0; i < m; ++i) {
            for (int j = 0; j < p; ++j) {
                c[i][j] = 0;
                for (int k = 0; k < n; ++k) c[i][j] += a[i][k] * b[k][j];
            }
        }
        return c;
    }

    static constexpr auto identity(value_t scalar = 1) {
        static_assert(m == n);
        mat_type result{};
        for (int i = 0; i < m; ++i) result[i][i] = scalar;
        return result;
    }
};

template <typename value_t>
inline auto perspective(value_t fovy, value_t aspect, value_t z_near, value_t z_far) {
    const value_t tan_half_fovy = std::tan(fovy / 2);

    mat<value_t, 4, 4> result{};
    result[0] = {value_t{1} / (aspect * tan_half_fovy), 0, 0, 0};
    result[1] = {0, value_t{1} / tan_half_fovy, 0, 0};
    result[2] = {0, 0, -(z_far + z_near) / (z_far - z_near), 0};
    result[3] = {0, 0, -(value_t{2} * z_far * z_near) / (z_far - z_near), -1};
    return result;
}

template <typename value_t>
constexpr auto translate(const mat<value_t, 4, 4> & m, const vec<value_t, 3> & v) {
    mat<value_t, 4, 4> result = m;
    result[3] += m[0] * v[0] + m[1] * v[1] + m[2] * v[2];
    return result;
}

template <typename value_t>
inline auto rotate(const mat<value_t, 4, 4> & m, value_t a, const vec<value_t, 3> & v) {
    const value_t c = std::cos(a);
    const value_t s = std::sin(a);

    vec<value_t, 3> axis = normalize(v);
    vec<value_t, 3> temp = axis * (-c + 1);

    mat<value_t, 3, 3> rotate;
    rotate[0][0] = c + temp[0] * axis[0];
    rotate[0][1] = temp[0] * axis[1] + s * axis[2];
    rotate[0][2] = temp[0] * axis[2] - s * axis[1];

    rotate[1][0] = temp[1] * axis[0] - s * axis[2];
    rotate[1][1] = c + temp[1] * axis[1];
    rotate[1][2] = temp[1] * axis[2] + s * axis[0];

    rotate[2][0] = temp[2] * axis[0] + s * axis[1];
    rotate[2][1] = temp[2] * axis[1] - s * axis[0];
    rotate[2][2] = c + temp[2] * axis[2];

    mat<value_t, 4, 4> result{
        m[0] * rotate[0][0] + m[1] * rotate[0][1] + m[2] * rotate[0][2],
        m[0] * rotate[1][0] + m[1] * rotate[1][1] + m[2] * rotate[1][2],
        m[0] * rotate[2][0] + m[1] * rotate[2][1] + m[2] * rotate[2][2],
        m[3],
    };
    return result;
}

template <typename value_t>
constexpr auto scale(const mat<value_t, 4, 4> & m, const vec<value_t, 3> & v) {
    mat<value_t, 4, 4> result = m;
    result[0] *= v[0];
    result[1] *= v[1];
    result[2] *= v[2];
    return result;
}
