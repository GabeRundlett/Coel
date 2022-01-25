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

    Buffer::Buffer(VkDevice device, VkPhysicalDeviceMemoryProperties vk_memory_properties, void *data_ptr, size_t data_size, VkBufferUsageFlags usage) {
        device_handle = device;
        VkBufferCreateInfo buffer_ci{
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = data_size,
            .usage = usage,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        };
        vkCreateBuffer(device_handle, &buffer_ci, nullptr, &handle);
        VkMemoryRequirements memory_requirements;
        vkGetBufferMemoryRequirements(device_handle, handle, &memory_requirements);
        uint32_t type_index;
        memory_type_from_properties(vk_memory_properties, memory_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &type_index);
        VkMemoryAllocateInfo alloc_info{
            .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
            .allocationSize = memory_requirements.size,
            .memoryTypeIndex = type_index,
        };
        vkAllocateMemory(device_handle, &alloc_info, nullptr, &memory);
        vkBindBufferMemory(device_handle, handle, memory, 0);
        void *mapped_region;
        vkMapMemory(device_handle, memory, 0, data_size, 0, &mapped_region);
        memcpy(mapped_region, data_ptr, data_size);
        vkUnmapMemory(device_handle, memory);
    }

    Buffer::~Buffer() {
        vkDestroyBuffer(device_handle, handle, nullptr);
        vkFreeMemory(device_handle, memory, nullptr);
    }

    void Buffer::upload(void *data_ptr, size_t data_size) {
        void *mapped_region;
        vkMapMemory(device_handle, memory, 0, data_size, 0, &mapped_region);
        memcpy(mapped_region, data_ptr, data_size);
        vkUnmapMemory(device_handle, memory);
    }

    void Buffer::bind_vbo(VkCommandBuffer cmd) {
        VkDeviceSize offsets[]{0};
        vkCmdBindVertexBuffers(cmd, 0, 1, &handle, offsets);
    }

    void Buffer::bind_ibo(VkCommandBuffer cmd) {
        vkCmdBindIndexBuffer(cmd, handle, 0, VK_INDEX_TYPE_UINT32);
    }
} // namespace coel::vulkan