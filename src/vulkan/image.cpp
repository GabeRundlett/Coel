#include <coel/vulkan/core.hpp>

namespace coel::vulkan {
    static bool memory_type_from_properties(const auto &memory_properties, uint32_t typeBits, VkFlags requirements_mask, uint32_t *typeIndex) {
        for (uint32_t i = 0; i < VK_MAX_MEMORY_TYPES; i++) {
            if ((typeBits & 1) == 1) {
                if ((memory_properties.memoryTypes[i].propertyFlags & requirements_mask) == requirements_mask) {
                    *typeIndex = i;
                    return true;
                }
            }
            typeBits >>= 1;
        }
        return false;
    };

    Image::Image(VkDevice device, const VkPhysicalDeviceMemoryProperties &vk_memory_properties, uint32_t sx, uint32_t sy) {
        device_handle = device;
        size_x = sx, size_y = sy;
        auto tiling = VK_IMAGE_TILING_LINEAR;
        VkFlags required_props = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        VkImageUsageFlags usage = VK_IMAGE_USAGE_SAMPLED_BIT;
        format = VK_FORMAT_R8G8B8A8_UNORM;
        const VkImageCreateInfo image_ci = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .imageType = VK_IMAGE_TYPE_2D,
            .format = format,
            .extent = {size_x, size_y, 1},
            .mipLevels = 1,
            .arrayLayers = 1,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .tiling = tiling,
            .usage = usage,
            .initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED,
        };
        vkCreateImage(device_handle, &image_ci, nullptr, &handle);
        VkMemoryRequirements mem_reqs;
        vkGetImageMemoryRequirements(device_handle, handle, &mem_reqs);
        mem_alloc_info = {};
        mem_alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        mem_alloc_info.pNext = nullptr;
        mem_alloc_info.allocationSize = mem_reqs.size;
        mem_alloc_info.memoryTypeIndex = 0;
        memory_type_from_properties(vk_memory_properties, mem_reqs.memoryTypeBits, required_props, &mem_alloc_info.memoryTypeIndex);
        vkAllocateMemory(device_handle, &mem_alloc_info, nullptr, &memory);
        vkBindImageMemory(device_handle, handle, memory, 0);
        const VkImageSubresource subres = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .mipLevel = 0,
            .arrayLayer = 0,
        };
        VkSubresourceLayout layout;
        vkGetImageSubresourceLayout(device_handle, handle, &subres, &layout);
    }

    Image::~Image() {
        vkDestroySampler(device_handle, sampler, nullptr);
        vkDestroyImageView(device_handle, view, nullptr);
        vkFreeMemory(device_handle, memory, nullptr);
        vkDestroyImage(device_handle, handle, nullptr);
    }

    void Image::upload(VkCommandBuffer cmd, const std::vector<uint8_t> pixels, uint32_t channels) {
        uint8_t *mapped_ptr;
        vkMapMemory(device_handle, memory, 0, mem_alloc_info.allocationSize, 0, reinterpret_cast<void **>(&mapped_ptr));
        switch (channels) {
        case 3: {
            for (size_t i = 0; i < pixels.size() / 3; ++i) {
                mapped_ptr[i * 4 + 0] = pixels[i * 3 + 0];
                mapped_ptr[i * 4 + 1] = pixels[i * 3 + 1];
                mapped_ptr[i * 4 + 2] = pixels[i * 3 + 2];
                mapped_ptr[i * 4 + 3] = 255;
            }
        } break;
        case 4: {
            for (size_t i = 0; i < pixels.size(); ++i)
                mapped_ptr[i] = pixels[i];
        } break;
        }
        vkUnmapMemory(device_handle, memory);

        VkImageMemoryBarrier image_memory_barrier = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .pNext = nullptr,
            .srcAccessMask = VK_ACCESS_NONE_KHR,
            .dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_INPUT_ATTACHMENT_READ_BIT,
            .oldLayout = VK_IMAGE_LAYOUT_PREINITIALIZED,
            .newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image = handle,
            .subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1},
        };
        vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &image_memory_barrier);

        const VkSamplerCreateInfo sampler_ci = {
            .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
            .pNext = nullptr,
            .magFilter = VK_FILTER_NEAREST,
            .minFilter = VK_FILTER_NEAREST,
            .mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST,
            .addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
            .addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
            .addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
            .mipLodBias = 0.0f,
            .anisotropyEnable = VK_FALSE,
            .maxAnisotropy = 1,
            .compareOp = VK_COMPARE_OP_NEVER,
            .minLod = 0.0f,
            .maxLod = 0.0f,
            .borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE,
            .unnormalizedCoordinates = VK_FALSE,
        };

        VkImageViewCreateInfo view_ci = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .image = VK_NULL_HANDLE,
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = format,
            .components =
                {
                    VK_COMPONENT_SWIZZLE_IDENTITY,
                    VK_COMPONENT_SWIZZLE_IDENTITY,
                    VK_COMPONENT_SWIZZLE_IDENTITY,
                    VK_COMPONENT_SWIZZLE_IDENTITY,
                },
            .subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1},
        };
        vkCreateSampler(device_handle, &sampler_ci, nullptr, &sampler);
        view_ci.image = handle;
        vkCreateImageView(device_handle, &view_ci, nullptr, &view);
    }
} // namespace coel::vulkan
