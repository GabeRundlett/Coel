#pragma once

#include <vulkan/vulkan.h>

namespace coel::vulkan {
    struct Buffer {
        VkDevice device_handle;

        VkBuffer handle;
        VkDeviceMemory memory;

        COEL_EXPORT Buffer(VkDevice device, VkPhysicalDeviceMemoryProperties vk_memory_properties, void *data_ptr, size_t data_size, VkBufferUsageFlags usage);
        COEL_EXPORT ~Buffer();
        COEL_EXPORT void bind_vbo(VkCommandBuffer cmd);
        COEL_EXPORT void bind_ibo(VkCommandBuffer cmd);
    };
}
