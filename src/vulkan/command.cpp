#include <coel/vulkan/core.hpp>

namespace coel::vulkan {
    CommandPool::CommandPool(VkDevice device, uint32_t queue_family_index) {
        device_handle = device;
        const VkCommandPoolCreateInfo cmd_pool_ci = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .queueFamilyIndex = queue_family_index,
        };
        vkCreateCommandPool(device_handle, &cmd_pool_ci, nullptr, &handle);
    }

    CommandPool::~CommandPool() {
        vkDestroyCommandPool(device_handle, handle, nullptr);
    }

    CommandBuffer CommandPool::get_command_buffer() {
        CommandBuffer result{.device_handle = device_handle, .command_pool = handle};
        const VkCommandBufferAllocateInfo cmd_alloc_info = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .pNext = NULL,
            .commandPool = handle,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1,
        };
        vkAllocateCommandBuffers(device_handle, &cmd_alloc_info, &result.handle);
        return result;
    }

    CommandBuffer::~CommandBuffer() {
        vkFreeCommandBuffers(device_handle, command_pool, 1, &handle);
    }

    void CommandBuffer::begin() {
        VkCommandBufferBeginInfo cmd_buf_info = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .pNext = NULL,
            .flags = 0,
            .pInheritanceInfo = NULL,
        };
        vkBeginCommandBuffer(handle, &cmd_buf_info);
    }

    void CommandBuffer::end() {
        vkEndCommandBuffer(handle);
    }

    void CommandBuffer::submit_blocking(VkQueue queue) {
        VkFence fence;
        VkFenceCreateInfo fence_ci = {
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
        };
        vkCreateFence(device_handle, &fence_ci, nullptr, &fence);
        const VkCommandBuffer cmd_bufs[] = {handle};
        VkSubmitInfo submit_info = {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .pNext = nullptr,
            .waitSemaphoreCount = 0,
            .pWaitSemaphores = nullptr,
            .pWaitDstStageMask = nullptr,
            .commandBufferCount = 1,
            .pCommandBuffers = cmd_bufs,
            .signalSemaphoreCount = 0,
            .pSignalSemaphores = nullptr,
        };
        vkQueueSubmit(queue, 1, &submit_info, fence);
        vkWaitForFences(device_handle, 1, &fence, VK_TRUE, UINT64_MAX);
        vkDestroyFence(device_handle, fence, nullptr);
    }
} // namespace coel::vulkan