#pragma once

#include <vulkan/vulkan.h>
#include <coel/window.hpp>

namespace coel::vulkan {
    struct Surface {
        VkInstance instance_handle;

        VkSurfaceKHR handle;
        VkSurfaceFormatKHR format;

        COEL_EXPORT Surface() = default;
        COEL_EXPORT Surface(VkInstance instance, WindowHandle window);
        COEL_EXPORT ~Surface();

        Surface(const Surface &) = delete;
        Surface &operator=(const Surface &) = delete;
        Surface(Surface &&other) {
            *this = std::move(other);
        }
        Surface &operator=(Surface &&other) {
            instance_handle = other.instance_handle;
            handle = other.handle;
            format = other.format;
            other.instance_handle = nullptr;
            other.handle = nullptr;
            other.format = {};
            return *this;
        }

        COEL_EXPORT void select_format(VkPhysicalDevice physical_device);
    };
} // namespace coel::vulkan
