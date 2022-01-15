#pragma once

#include <cstdint>
#include <numbers>

constexpr auto radians(auto angle_degrees) {
    return angle_degrees * std::numbers::pi_v<decltype(angle_degrees)> / 180;
}
