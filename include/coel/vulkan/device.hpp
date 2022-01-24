#pragma once

#include <vulkan/vulkan.h>
#include <vector>

namespace coel::vulkan {
    using PhysicalDevice = VkPhysicalDevice;
    COEL_EXPORT PhysicalDevice choose_physical_device(VkInstance instance);
    COEL_EXPORT uint32_t select_graphics_and_present_queue(VkPhysicalDevice physical_device, VkSurfaceKHR surface);

    struct Device {
        VkInstance instance_handle;

        VkDevice handle;
        std::vector<const char *> extension_names;
        std::vector<const char *> enabled_layers;

        std::vector<VkQueue> queues;

        COEL_EXPORT Device(VkInstance instance, VkPhysicalDevice physical_device, const std::vector<uint32_t> &queue_family_indices);
        COEL_EXPORT ~Device();

        COEL_EXPORT void wait_idle();
    };
} // namespace coel::vulkan
