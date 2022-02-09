#include <coel/vulkan/core.hpp>
#include <stdexcept>

namespace coel::vulkan {
    void Swapchain::recreate_swapchain() {
        vkDeviceWaitIdle(device_handle);
        recreate_cleanup();
        VkSwapchainKHR old_swapchain = handle;
        VkSurfaceCapabilitiesKHR surface_capabilities;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device_handle, surface.handle, &surface_capabilities);
        uint32_t present_mode_count;
        vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device_handle, surface.handle, &present_mode_count, nullptr);
        std::vector<VkPresentModeKHR> present_modes;
        present_modes.resize(present_mode_count);
        vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device_handle, surface.handle, &present_mode_count, present_modes.data());
        VkExtent2D swapchain_extent;
        if (surface_capabilities.currentExtent.width == 0xFFFFFFFF) {
            swapchain_extent.width = size_x;
            swapchain_extent.height = size_y;
            if (swapchain_extent.width < surface_capabilities.minImageExtent.width)
                swapchain_extent.width = surface_capabilities.minImageExtent.width;
            else if (swapchain_extent.width > surface_capabilities.maxImageExtent.width)
                swapchain_extent.width = surface_capabilities.maxImageExtent.width;
            if (swapchain_extent.height < surface_capabilities.minImageExtent.height)
                swapchain_extent.height = surface_capabilities.minImageExtent.height;
            else if (swapchain_extent.height > surface_capabilities.maxImageExtent.height)
                swapchain_extent.height = surface_capabilities.maxImageExtent.height;
        } else {
            swapchain_extent = surface_capabilities.currentExtent;
            size_x = surface_capabilities.currentExtent.width;
            size_y = surface_capabilities.currentExtent.height;
        }
        if (surface_capabilities.maxImageExtent.width == 0 || surface_capabilities.maxImageExtent.height == 0) {
            prepared = false;
            return;
        }
        present_mode = VK_PRESENT_MODE_FIFO_KHR;
        uint32_t desired_image_n = Swapchain::FRAMES_N;
        if (desired_image_n < surface_capabilities.minImageCount)
            desired_image_n = surface_capabilities.minImageCount;
        if ((surface_capabilities.maxImageCount > 0) && (desired_image_n > surface_capabilities.maxImageCount))
            desired_image_n = surface_capabilities.maxImageCount;
        VkSurfaceTransformFlagBitsKHR preTransform;
        if (surface_capabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
            preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
        else
            preTransform = surface_capabilities.currentTransform;
        VkCompositeAlphaFlagBitsKHR composite_alpha =
            VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        VkCompositeAlphaFlagBitsKHR composite_alpha_flags[4] = {
            VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
            VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
            VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
            VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
        };
        for (auto alpha_flag : composite_alpha_flags) {
            if (surface_capabilities.supportedCompositeAlpha & alpha_flag) {
                composite_alpha = alpha_flag;
                break;
            }
        }
        VkSwapchainCreateInfoKHR swapchain_ci = {
            .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
            .pNext = nullptr,
            .surface = surface.handle,
            .minImageCount = desired_image_n,
            .imageFormat = surface.format.format,
            .imageColorSpace = surface.format.colorSpace,
            .imageExtent = {
                .width = swapchain_extent.width,
                .height = swapchain_extent.height,
            },
            .imageArrayLayers = 1,
            .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
            .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .queueFamilyIndexCount = 0,
            .pQueueFamilyIndices = nullptr,
            .preTransform = preTransform,
            .compositeAlpha = composite_alpha,
            .presentMode = present_mode,
            .clipped = true,
            .oldSwapchain = old_swapchain,
        };
        vkCreateSwapchainKHR(device_handle, &swapchain_ci, nullptr, &handle);
        if (old_swapchain != VK_NULL_HANDLE)
            vkDestroySwapchainKHR(device_handle, old_swapchain, nullptr);
        uint32_t image_count;
        vkGetSwapchainImagesKHR(device_handle, handle, &image_count, nullptr);
        std::vector<VkImage> swapchain_images;
        swapchain_images.resize(image_count);
        vkGetSwapchainImagesKHR(device_handle, handle, &image_count, swapchain_images.data());
        image_resources.resize(image_count);
        for (uint32_t i = 0; i < image_resources.size(); i++) {
            VkImageViewCreateInfo color_image_view = {
                .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .viewType = VK_IMAGE_VIEW_TYPE_2D,
                .format = surface.format.format,
                .components = {
                    .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .a = VK_COMPONENT_SWIZZLE_IDENTITY,
                },
                .subresourceRange = {
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .baseMipLevel = 0,
                    .levelCount = 1,
                    .baseArrayLayer = 0,
                    .layerCount = 1,
                },
            };
            image_resources[i].image = swapchain_images[i];
            color_image_view.image = image_resources[i].image;
            vkCreateImageView(device_handle, &color_image_view, nullptr, &image_resources[i].view);
        }

        const VkCommandBufferAllocateInfo cmd = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .pNext = nullptr,
            .commandPool = command_pool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1,
        };
        for (uint32_t i = 0; i < image_resources.size(); i++) {
            vkAllocateCommandBuffers(device_handle, &cmd, &image_resources[i].cmd);
            image_resources[i].cmd_recorded = false;
        }

        if (!render_pass) {
            const VkAttachmentDescription attachments[1] = {
                {
                    .flags = 0,
                    .format = surface.format.format,
                    .samples = VK_SAMPLE_COUNT_1_BIT,
                    .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                    .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                    .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                    .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                    .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                    .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                },
                // {
                //     .flags = 0,
                //     .format = vk.swapchain.depth.format,
                //     .samples = VK_SAMPLE_COUNT_1_BIT,
                //     .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                //     .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                //     .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                //     .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                //     .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                //     .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                // },
            };
            const VkAttachmentReference color_reference = {
                .attachment = 0,
                .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            };
            // const VkAttachmentReference depth_reference = {
            //     .attachment = 1,
            //     .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
            // };
            const VkSubpassDescription subpass = {
                .flags = 0,
                .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
                .inputAttachmentCount = 0,
                .pInputAttachments = nullptr,
                .colorAttachmentCount = 1,
                .pColorAttachments = &color_reference,
                .pResolveAttachments = nullptr,
                .pDepthStencilAttachment = nullptr, // &depth_reference,
                .preserveAttachmentCount = 0,
                .pPreserveAttachments = nullptr,
            };
            VkSubpassDependency attachmentDependencies[1] = {
                {
                    .srcSubpass = VK_SUBPASS_EXTERNAL,
                    .dstSubpass = 0,
                    .srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
                    .dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
                    .srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
                    .dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
                    .dependencyFlags = 0,
                },
                // {
                //     .srcSubpass = VK_SUBPASS_EXTERNAL,
                //     .dstSubpass = 0,
                //     .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                //     .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                //     .srcAccessMask = 0,
                //     .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT,
                //     .dependencyFlags = 0,
                // },
            };
            const VkRenderPassCreateInfo renderpass_ci = {
                .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .attachmentCount = 1,
                .pAttachments = attachments,
                .subpassCount = 1,
                .pSubpasses = &subpass,
                .dependencyCount = 1,
                .pDependencies = attachmentDependencies,
            };
            vkCreateRenderPass(device_handle, &renderpass_ci, nullptr, &render_pass);
        }

        prepared = true;
        resized_flag = true;

        VkImageView attachments[1];
        // attachments[1] = vk.swapchain.depth.view;
        const VkFramebufferCreateInfo fb_info = {
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .pNext = nullptr,
            .renderPass = render_pass,
            .attachmentCount = 1,
            .pAttachments = attachments,
            .width = static_cast<uint32_t>(size_x),
            .height = static_cast<uint32_t>(size_y),
            .layers = 1,
        };
        uint32_t i;
        for (i = 0; i < image_resources.size(); i++) {
            attachments[0] = image_resources[i].view;
            vkCreateFramebuffer(device_handle, &fb_info, nullptr, &image_resources[i].framebuffer);
        }
    }
    void Swapchain::recreate_cleanup() {
        for (size_t i = 0; i < image_resources.size(); i++) {
            vkDestroyFramebuffer(device_handle, image_resources[i].framebuffer, nullptr);
            vkDestroyImageView(device_handle, image_resources[i].view, nullptr);
            vkFreeCommandBuffers(device_handle, command_pool, 1, &image_resources[i].cmd);

            image_resources[i].cmd_recorded = false;

            image_resources[i].framebuffer = VK_NULL_HANDLE;
            image_resources[i].view = VK_NULL_HANDLE;
            image_resources[i].cmd = VK_NULL_HANDLE;
        }
    }

    Swapchain::Swapchain(VkPhysicalDevice physical_device, Surface &surface, VkDevice device, uint32_t graphics_queue_family_index) : surface(surface) {
        device_handle = device;
        handle = nullptr;
        render_pass = nullptr;
        size_x = 0, size_y = 0;

        physical_device_handle = physical_device;

        // Sync stuff

        current_frame_index = 0;
        VkSemaphoreCreateInfo semaphore_ci = {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
        };
        VkFenceCreateInfo fence_ci = {
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            .pNext = nullptr,
            .flags = VK_FENCE_CREATE_SIGNALED_BIT,
        };
        for (uint32_t i = 0; i < FRAMES_N; i++) {
            vkCreateFence(device_handle, &fence_ci, nullptr, &frames[i].fence);
            vkCreateSemaphore(device_handle, &semaphore_ci, nullptr, &frames[i].image_acquired_semaphore);
            vkCreateSemaphore(device_handle, &semaphore_ci, nullptr, &frames[i].draw_complete_semaphore);
        }

        // Command pool

        const VkCommandPoolCreateInfo cmd_pool_ci = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .queueFamilyIndex = graphics_queue_family_index,
        };
        vkCreateCommandPool(device_handle, &cmd_pool_ci, nullptr, &command_pool);

        recreate_swapchain();
    }

    Swapchain::~Swapchain() {
        if (!handle)
            return;
        recreate_cleanup();
        if (render_pass)
            vkDestroyRenderPass(device_handle, render_pass, nullptr);

        for (size_t i = 0; i < frames.size(); i++) {
            vkDestroyFence(device_handle, frames[i].fence, nullptr);
            vkDestroySemaphore(device_handle, frames[i].image_acquired_semaphore, nullptr);
            vkDestroySemaphore(device_handle, frames[i].draw_complete_semaphore, nullptr);
        }
        vkDestroyCommandPool(device_handle, command_pool, nullptr);
        vkDestroySwapchainKHR(device_handle, handle, nullptr);
        handle = nullptr;
    }

    bool Swapchain::was_resized() {
        if (resized_flag) {
            resized_flag = false;
            return true;
        }
        return false;
    }

    void Swapchain::wait_for_frame() {
        if (!prepared)
            recreate_swapchain();

        vkWaitForFences(device_handle, 1, &frames[current_frame_index].fence, VK_TRUE, UINT64_MAX);
        vkResetFences(device_handle, 1, &frames[current_frame_index].fence);
        VkResult err;
        do {
            err = vkAcquireNextImageKHR(
                device_handle, handle, UINT64_MAX,
                frames[current_frame_index].image_acquired_semaphore,
                VK_NULL_HANDLE, &current_image_index);
            if (err == VK_ERROR_OUT_OF_DATE_KHR) {
                recreate_swapchain();
            } else if (err == VK_ERROR_SURFACE_LOST_KHR) {
                surface = Surface(surface.instance_handle, surface.window_handle);
                recreate_swapchain();
            } else if (err == VK_SUBOPTIMAL_KHR) {
                break;
            } else if (err != VK_SUCCESS) {
                throw std::runtime_error("Unexpected swapchain error");
            }
        } while (err != VK_SUCCESS);
    }

    void Swapchain::begin_renderpass(VkCommandBuffer cmd, const std::array<float, 4> &clear_col) {
        const VkClearValue clear_values[1] = {
            {.color{.float32 = {clear_col[0], clear_col[1], clear_col[2], clear_col[3]}}},
            // {.depthStencil = {1.0f, 0}},
        };
        const VkRenderPassBeginInfo rp_begin_info = {
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            .pNext = nullptr,
            .renderPass = render_pass,
            .framebuffer = image_resources[current_image_index].framebuffer,
            .renderArea{
                .offset{.x = 0, .y = 0},
                .extent{
                    .width = static_cast<uint32_t>(size_x),
                    .height = static_cast<uint32_t>(size_y),
                },
            },
            .clearValueCount = 2,
            .pClearValues = clear_values,
        };
        vkCmdBeginRenderPass(cmd, &rp_begin_info, VK_SUBPASS_CONTENTS_INLINE);
    }

    void Swapchain::present_and_swap(VkQueue present_queue) {
        if (!image_resources[current_image_index].cmd_recorded) {
            draw_cmd_func(image_resources[current_image_index].cmd);
            image_resources[current_image_index].cmd_recorded = true;
        }

        VkPipelineStageFlags pipe_stage_flags;
        VkSubmitInfo submit_info;
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.pNext = nullptr;
        submit_info.pWaitDstStageMask = &pipe_stage_flags;
        pipe_stage_flags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        submit_info.waitSemaphoreCount = 1;
        submit_info.pWaitSemaphores = &frames[current_frame_index].image_acquired_semaphore;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &image_resources[current_image_index].cmd;
        submit_info.signalSemaphoreCount = 1;
        submit_info.pSignalSemaphores = &frames[current_frame_index].draw_complete_semaphore;
        vkQueueSubmit(present_queue, 1, &submit_info, frames[current_frame_index].fence);
        VkPresentInfoKHR present = {
            .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .pNext = nullptr,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &frames[current_frame_index].draw_complete_semaphore,
            .swapchainCount = 1,
            .pSwapchains = &handle,
            .pImageIndices = &current_image_index,
        };
        auto err = vkQueuePresentKHR(present_queue, &present);
        current_frame_index += 1;
        current_frame_index %= FRAMES_N;
        if (err == VK_ERROR_OUT_OF_DATE_KHR) {
            recreate_swapchain();
        } else if (err == VK_SUBOPTIMAL_KHR) {
            VkSurfaceCapabilitiesKHR surface_capabilities;
            vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device_handle, surface.handle, &surface_capabilities);
            if (surface_capabilities.currentExtent.width != size_x ||
                surface_capabilities.currentExtent.height != size_y) {
                recreate_swapchain();
            }
        } else if (err == VK_ERROR_SURFACE_LOST_KHR) {
            surface = Surface(surface.instance_handle, surface.window_handle);
            recreate_swapchain();
        } else if (err != VK_SUCCESS) {
            throw std::runtime_error("Unexpected swapchain present error");
        }
    }
} // namespace coel::vulkan
