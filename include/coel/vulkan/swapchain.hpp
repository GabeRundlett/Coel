#pragma once

#include <vulkan/vulkan.h>
#include <cstdint>
#include <vector>
#include <array>

namespace coel::vulkan {
    struct Swapchain {
        VkDevice device_handle;

        VkPhysicalDevice physical_device_handle;
        VkSurfaceKHR surface_handle;
        VkSurfaceFormatKHR surface_format;

        VkSwapchainKHR handle;
        VkPresentModeKHR present_mode;
        VkCommandPool command_pool;
        VkRenderPass render_pass;

        uint32_t size_x, size_y;
        bool prepared;

        struct ImageResources {
            VkImage image;
            VkCommandBuffer cmd;
            VkImageView view;
            VkFramebuffer framebuffer;
            bool cmd_recorded;
        };
        std::vector<ImageResources> image_resources;
        uint32_t current_image_index;

        struct Frame {
            VkFence fence;
            VkSemaphore image_acquired_semaphore;
            VkSemaphore draw_complete_semaphore;
        };
        static constexpr size_t FRAMES_N = 3;
        std::array<Frame, FRAMES_N> frames;
        size_t current_frame_index;

        COEL_EXPORT Swapchain(VkPhysicalDevice physical_device, VkSurfaceKHR surface, VkSurfaceFormatKHR format, VkDevice device, uint32_t graphics_queue_family_index);
        COEL_EXPORT ~Swapchain();

        Swapchain(const Swapchain &) = delete;
        Swapchain &operator=(const Swapchain &) = delete;
        Swapchain(Swapchain &&other) = delete;
        Swapchain &operator=(Swapchain &&other) = delete;

        COEL_EXPORT void wait_for_frame();
        COEL_EXPORT void present_and_swap(VkQueue present_queue);
        COEL_EXPORT void begin_renderpass(VkCommandBuffer cmd, const std::array<float, 4> &clear_col);

      private:
        void recreate_cleanup();
        void recreate_swapchain();
    };
} // namespace coel::vulkan