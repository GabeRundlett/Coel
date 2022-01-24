#include <coel/vulkan/core.hpp>

namespace coel::vulkan {
    Surface::Surface(VkInstance instance, WindowHandle window) {
        instance_handle = instance;

#if COEL_USE_WIN32
        VkWin32SurfaceCreateInfoKHR surface_ci{
            .sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
            .pNext = nullptr,
            .flags = 0,
            .hinstance = GetModuleHandleA(nullptr),
            .hwnd = window,
        };
        vkCreateWin32SurfaceKHR(instance_handle, &surface_ci, nullptr, &handle);
#elif COEL_USE_X11
        VkXlibSurfaceCreateInfoKHR surface_ci{
            .sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR,
            .pNext = nullptr,
            .flags = 0,
            .dpy = nullptr,
            .window = window,
        };
        vkCreateXlibSurfaceKHR(instance_handle, &surface_ci, nullptr, &handle);
#endif
    }

    Surface::~Surface() {
        if (!handle)
            return;
        vkDestroySurfaceKHR(instance_handle, handle, nullptr);
        handle = nullptr;
    }

    void Surface::select_format(VkPhysicalDevice physical_device) {
        uint32_t format_count = 0;
        vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, handle, &format_count, nullptr);
        std::vector<VkSurfaceFormatKHR> surface_formats;
        surface_formats.resize(format_count);
        vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, handle, &format_count, surface_formats.data());
        auto pick_surface_format = [](const std::vector<VkSurfaceFormatKHR> &surface_formats) -> VkSurfaceFormatKHR {
            for (const auto &surface_format : surface_formats) {
                if (surface_format.format == VK_FORMAT_R8G8B8A8_UNORM ||
                    surface_format.format == VK_FORMAT_B8G8R8A8_UNORM ||
                    surface_format.format == VK_FORMAT_A2B10G10R10_UNORM_PACK32 ||
                    surface_format.format == VK_FORMAT_A2R10G10B10_UNORM_PACK32 ||
                    surface_format.format == VK_FORMAT_R16G16B16A16_SFLOAT) {
                    return surface_format;
                }
            }
            return surface_formats[0];
        };
        format = pick_surface_format(surface_formats);
    }
} // namespace coel::vulkan
