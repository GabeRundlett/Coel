#pragma once

#include <vulkan/vulkan.h>
#include <coel/window.hpp>

namespace coel::vulkan {
    struct Surface {
        VkInstance instance_handle;
        WindowHandle window_handle;

        VkSurfaceKHR handle = nullptr;
        VkSurfaceFormatKHR format;

        COEL_EXPORT Surface() = default;
        COEL_EXPORT Surface(VkInstance instance, WindowHandle window);
        COEL_EXPORT ~Surface();

        Surface(const Surface &) = delete;
        Surface &operator=(const Surface &) = delete;
        Surface(Surface &&other) {
            *this = std::move(other);
        }
        COEL_EXPORT Surface &operator=(Surface &&other);

        COEL_EXPORT void select_format(VkPhysicalDevice physical_device);
    };
} // namespace coel::vulkan
