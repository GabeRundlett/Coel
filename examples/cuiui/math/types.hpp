#pragma once

#include "vector.hpp"
#include "rect.hpp"
#include "matrix.hpp"

// scalars

using i8  = std::int8_t;
using u8  = std::uint8_t;
using i16 = std::int16_t;
using u16 = std::uint16_t;
using i32 = std::int32_t;
using u32 = std::uint32_t;
using i64 = std::int64_t;
using u64 = std::uint64_t;
using uxx = std::uintptr_t;
using ixx = std::intptr_t;
using f32 = float;
using f64 = double;
using f80 = long double;

// vectors

using i32vec2 = vec<i32, 2>;
using u32vec2 = vec<u32, 2>;
using f32vec2 = vec<f32, 2>;
using f64vec2 = vec<f64, 2>;

using i32vec3 = vec<i32, 3>;
using u32vec3 = vec<u32, 3>;
using f32vec3 = vec<f32, 3>;
using f64vec3 = vec<f64, 3>;

using i32vec4 = vec<i32, 4>;
using u32vec4 = vec<u32, 4>;
using f32vec4 = vec<f32, 4>;
using f64vec4 = vec<f64, 4>;

// rects

using i32rect = rect<i32vec2>;
using u32rect = rect<u32vec2>;
using f32rect = rect<f32vec2>;
using f64rect = rect<f64vec2>;

// matrices

using f32mat4 = mat<f32, 4, 4>;
