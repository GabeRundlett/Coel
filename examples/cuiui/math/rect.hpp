#pragma once
#include "vector.hpp"

template <typename point_t> struct rect {
    using point_type = point_t;
    using rect_type  = rect<point_type>;
    point_type p0, p1;
};
