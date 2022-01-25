#include <coel/vulkan/core.hpp>

#include <stdexcept>
#include <numeric>

namespace coel::vulkan {
    VkPhysicalDevice choose_physical_device(VkInstance instance) {
        uint32_t physical_device_n;
        vkEnumeratePhysicalDevices(instance, &physical_device_n, nullptr);
        std::vector<VkPhysicalDevice> physical_devices;
        physical_devices.resize(physical_device_n);
        vkEnumeratePhysicalDevices(instance, &physical_device_n, physical_devices.data());
        // TODO: add selection
        return physical_devices[0];
    }

    uint32_t select_graphics_and_present_queue(VkPhysicalDevice physical_device, VkSurfaceKHR surface) {
        uint32_t vk_queue_family_index = std::numeric_limits<uint32_t>::max();
        uint32_t queue_family_count = 0;
        std::vector<VkQueueFamilyProperties> queue_props;
        vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, nullptr);
        queue_props.resize(queue_family_count);
        vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, queue_props.data());
        std::vector<VkBool32> supports_present;
        supports_present.resize(queue_family_count);
        for (uint32_t i = 0; i < queue_family_count; i++)
            vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, i, surface, &supports_present[i]);
        for (uint32_t i = 0; i < queue_family_count; i++) {
            if ((queue_props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0 && supports_present[i] == VK_TRUE) {
                vk_queue_family_index = i;
                break;
            }
        }
        if (vk_queue_family_index == std::numeric_limits<uint32_t>::max())
            throw std::runtime_error("Failed to find suitable queue");

        return vk_queue_family_index;
    }

    VkPhysicalDeviceMemoryProperties get_physical_device_memory_properties(VkPhysicalDevice physical_device) {
        VkPhysicalDeviceMemoryProperties result;
        vkGetPhysicalDeviceMemoryProperties(physical_device, &result);
        return result;
    }

    Device::Device(VkInstance instance, VkPhysicalDevice physical_device, const std::vector<uint32_t> &queue_family_indices) {
        instance_handle = instance;

        float queue_priorities[1] = {0.0};
        std::vector<VkDeviceQueueCreateInfo> queue_cis;
        queue_cis.reserve(queue_family_indices.size());
        for (uint32_t i : queue_family_indices) {
            queue_cis.push_back({
                .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .queueFamilyIndex = i,
                .queueCount = 1,
                .pQueuePriorities = queue_priorities,
            });
        }
        extension_names.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
        VkDeviceCreateInfo device_ci = {
            .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .pNext = nullptr,
            .queueCreateInfoCount = static_cast<uint32_t>(queue_cis.size()),
            .pQueueCreateInfos = queue_cis.data(),
            .enabledLayerCount = static_cast<uint32_t>(enabled_layers.size()),
            .ppEnabledLayerNames = enabled_layers.data(),
            .enabledExtensionCount = static_cast<uint32_t>(extension_names.size()),
            .ppEnabledExtensionNames = extension_names.data(),
            .pEnabledFeatures = nullptr,
        };
        vkCreateDevice(physical_device, &device_ci, nullptr, &handle);
        volkLoadDevice(handle);

        queues.reserve(queue_family_indices.size());
        for (uint32_t i : queue_family_indices) {
            VkQueue queue_handle;
            vkGetDeviceQueue(handle, i, 0, &queue_handle);
            queues.push_back(queue_handle);
        }
    }

    Device::~Device() {
        vkDestroyDevice(handle, nullptr);
    }

    void Device::wait_idle() {
        vkDeviceWaitIdle(handle);
    }
} // namespace coel::vulkan
