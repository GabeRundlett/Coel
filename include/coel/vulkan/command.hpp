#pragma once

#include <vulkan/vulkan.h>
#include <vector>

namespace coel::vulkan {
    struct CommandBuffer {
        VkDevice device_handle;
        VkCommandPool command_pool;
        VkCommandBuffer handle;

        COEL_EXPORT ~CommandBuffer();

        COEL_EXPORT void begin();
        COEL_EXPORT void end();
        COEL_EXPORT void submit_blocking(VkQueue queue);
    };

    struct CommandPool {
        VkDevice device_handle;
        VkCommandPool handle;

        COEL_EXPORT CommandPool(VkDevice device, uint32_t queue_family_index);
        COEL_EXPORT ~CommandPool();

        COEL_EXPORT CommandBuffer get_command_buffer();
    };
} // namespace coel::vulkan
