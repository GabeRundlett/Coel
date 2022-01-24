#include <coel/vulkan/core.hpp>

#include <iostream>

namespace coel::vulkan {
    Instance::Instance() {
        volkInitialize();
        
        const VkApplicationInfo app_info = {
            .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
            .pNext = nullptr,
            .pApplicationName = "Vulkan App",
            .applicationVersion = 0,
            .pEngineName = "coel",
            .engineVersion = 0,
            .apiVersion = VK_API_VERSION_1_0,
        };

        enabled_layers.push_back("VK_LAYER_KHRONOS_validation");
        extension_names.push_back(VK_KHR_SURFACE_EXTENSION_NAME);

#if COEL_USE_WIN32
        extension_names.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#elif COEL_USE_X11
        extension_names.push_back(VK_KHR_XLIB_SURFACE_EXTENSION_NAME);
#elif COEL_USE_NULL
        // no surface extension
#endif

        {
            auto check_layers = [](auto &&required_names, auto &&layer_props) -> bool {
                for (auto &required_layer_name : required_names) {
                    bool found = false;
                    for (auto &existing_layer_prop : layer_props) {
                        if (!strcmp(required_layer_name, existing_layer_prop.layerName)) {
                            found = true;
                            break;
                        }
                    }
                    if (!found) {
                        std::cerr << "Cannot find layer: " << required_layer_name << std::endl;
                        return false;
                    }
                }
                return true;
            };

            std::vector<VkLayerProperties> instance_layers;
            uint32_t instance_layer_count;
            vkEnumerateInstanceLayerProperties(&instance_layer_count, nullptr);
            instance_layers.resize(instance_layer_count);
            vkEnumerateInstanceLayerProperties(&instance_layer_count, instance_layers.data());
            check_layers(enabled_layers, instance_layers);
        }

        VkInstanceCreateInfo instance_ci = {
            .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .pNext = nullptr,
            .pApplicationInfo = &app_info,
            .enabledLayerCount = static_cast<uint32_t>(enabled_layers.size()),
            .ppEnabledLayerNames = enabled_layers.data(),
            .enabledExtensionCount = static_cast<uint32_t>(extension_names.size()),
            .ppEnabledExtensionNames = extension_names.data(),
        };
        vkCreateInstance(&instance_ci, nullptr, &handle);

        volkLoadInstance(handle);
    }

    Instance::~Instance() {
        vkDestroyInstance(handle, nullptr);
    }
} // namespace coel::vulkan
