#pragma once

#include <vulkan/vulkan.h>

namespace coel::vulkan {
    struct GraphicsPipeline {
        VkDevice device_handle;

        VkPipeline handle;
        VkPipelineLayout layout;
        VkPipelineCache cache;
        VkShaderModule vert_shader_module;
        VkShaderModule frag_shader_module;

        struct Config {
            VkDevice device_handle;
            VkRenderPass render_pass;
            const char *const vert_src;
            const char *const frag_src;
            std::vector<VkVertexInputBindingDescription> bindings;
            std::vector<VkVertexInputAttributeDescription> attribs;
        };

        COEL_EXPORT GraphicsPipeline(const Config &config);
        COEL_EXPORT ~GraphicsPipeline();

        COEL_EXPORT void bind(VkCommandBuffer cmd);
    };
} // namespace coel::vulkan
