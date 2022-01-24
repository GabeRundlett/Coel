#pragma once

#include <vulkan/vulkan.h>
#include <vector>

namespace coel::vulkan {
    struct Instance {
        VkInstance handle;
        std::vector<const char *> extension_names;
        std::vector<const char *> enabled_layers;

        COEL_EXPORT Instance();
        COEL_EXPORT ~Instance();
    };
} // namespace coel::vulkan
