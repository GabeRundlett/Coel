#pragma once

#if COEL_USE_WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#elif COEL_USE_X11
#define VK_USE_PLATFORM_XLIB_KHR
#endif
#include <vulkan/vulkan.h>
#include <vector>

namespace coel::vulkan {
    struct Instance {
        VkInstance handle;
        std::vector<const char *> extension_names;
        std::vector<const char *> enabled_layers;
    };

    struct Device {
        VkDevice handle;
        std::vector<const char *> extension_names;
        std::vector<const char *> enabled_layers;
    };

    struct Surface {
        VkSurfaceKHR handle;
        VkSurfaceFormatKHR format;
    };

    struct Swapchain {
        struct ImageResources {
            VkImage image;
            VkCommandBuffer cmd;
            VkCommandBuffer graphics_to_present_cmd;
            VkImageView view;
            VkFramebuffer framebuffer;
        };

        struct Depth {
            VkFormat format;
            VkImage image;
            VkMemoryAllocateInfo mem_alloc;
            VkDeviceMemory mem;
            VkImageView view;
        };

        VkSwapchainKHR handle;
        uint32_t size_x, size_y;
        VkPresentModeKHR present_mode;
        std::vector<ImageResources> image_resources;
        Depth depth;
    };

    struct Pipeline {
        VkPipeline handle;
        VkPipelineLayout layout;
        VkPipelineCache cache;
        VkShaderModule vert_shader_module;
        VkShaderModule frag_shader_module;
        bool valid;
    };
} // namespace coel::vulkan
