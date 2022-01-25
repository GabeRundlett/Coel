#pragma once

#include <vulkan/vulkan.h>
#include <vector>

namespace coel::vulkan {
    struct Image {
        VkDevice device_handle;

        VkImage handle;
        VkDeviceMemory memory;
        VkSampler sampler;
        VkImageView view;
        VkFormat format;
        VkMemoryAllocateInfo mem_alloc_info;
        uint32_t size_x;
        uint32_t size_y;
        
        COEL_EXPORT Image(VkDevice device, const VkPhysicalDeviceMemoryProperties &vk_memory_properties, uint32_t size_x, uint32_t size_y);
        COEL_EXPORT ~Image();

        COEL_EXPORT void upload(VkCommandBuffer cmd, const std::vector<uint8_t> pixels, uint32_t channels);
    };
} // namespace coel::vulkan
