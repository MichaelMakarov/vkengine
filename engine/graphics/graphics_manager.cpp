#include "graphics_manager.hpp"
#include "graphics_error.hpp"

#include <cstddef>
#include <fstream>
#include <vector>

namespace {

    void *allocate_memory(void *data_ptr, size_t size, size_t alignment, VkSystemAllocationScope scope) {
        return nullptr;
    }

    void *reallocate_memory(void *pUserData, void *pOriginal, size_t size, size_t alignment, VkSystemAllocationScope allocationScope) {
        return nullptr;
    }

    void free_memory(void *pUserData, void *pMemory) {
    }

    void
    internal_allocation(void *pUserData, size_t size, VkInternalAllocationType allocationType, VkSystemAllocationScope allocationScope) {
    }

    void internal_free(void *pUserData, size_t size, VkInternalAllocationType allocationType, VkSystemAllocationScope allocationScope) {
    }

    /// TODO: think about approach and necessity to use this
    VkAllocationCallbacks allocation_callbacks{
        .pfnAllocation = &allocate_memory,
        .pfnReallocation = &reallocate_memory,
        .pfnFree = &free_memory,
        .pfnInternalAllocation = &internal_allocation,
        .pfnInternalFree = &internal_free,
    };

    VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT msg_severity,
                                                  VkDebugUtilsMessageTypeFlagsEXT /*msg_type*/,
                                                  const VkDebugUtilsMessengerCallbackDataEXT *callback_data,
                                                  void * /*user_data*/) {
        if (msg_severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
            info_println("VK debug messenger: {}", callback_data->pMessage);
        }
        return VK_FALSE;
    }

} // namespace

shared_ptr_of<VkInstance>
GraphicsManager::make_instance(char const *app_name, std::span<char const *const> extensions, std::span<char const *const> layers) {
    VkApplicationInfo app_info{
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = app_name,
        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
        .pEngineName = "VK_ENGINE",
        .engineVersion = VK_MAKE_VERSION(1, 0, 0),
        .apiVersion = VK_API_VERSION_1_2,
    };
    VkInstanceCreateInfo instance_info{
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &app_info,
        .enabledLayerCount = static_cast<uint32_t>(layers.size()),
        .ppEnabledLayerNames = layers.data(),
        .enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
        .ppEnabledExtensionNames = extensions.data(),
    };
    VkInstance instance;
    vk_assert(vkCreateInstance(&instance_info, nullptr, &instance), "Failed to create instance.");
    return shared_ptr_of<VkInstance>(instance, [](VkInstance instance) {
        debug_println("delete instance");
        vkDestroyInstance(instance, nullptr);
    });
}

unique_ptr_of<VkDebugUtilsMessengerEXT> GraphicsManager::make_debug_messenger(shared_ptr_of<VkInstance> instance) {
    VkDebugUtilsMessageSeverityFlagsEXT severity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                                   VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                                   VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    VkDebugUtilsMessageTypeFlagsEXT type = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                           VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    VkDebugUtilsMessengerCreateInfoEXT info{
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
        .messageSeverity = severity,
        .messageType = type,
        .pfnUserCallback = &debug_callback,
        .pUserData = nullptr,
    };
    auto func =
        reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance.get(), "vkCreateDebugUtilsMessengerEXT"));
    VkResult result = VK_ERROR_EXTENSION_NOT_PRESENT;
    VkDebugUtilsMessengerEXT messenger;
    if (func) {
        result = func(instance.get(), &info, nullptr, &messenger);
    }
    vk_assert(result, "failed to set debug messenger");
    return unique_ptr_of<VkDebugUtilsMessengerEXT>(messenger, [instance](VkDebugUtilsMessengerEXT messenger) {
        debug_println("delete debug utils messenger");
        auto func =
            reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance.get(), "vkDestroyDebugUtilsMessengerEXT"));
        if (func) {
            func(instance.get(), messenger, nullptr);
        }
    });
}

unique_ptr_of<VkSurfaceKHR> GraphicsManager::make_surface(shared_ptr_of<VkInstance> instance, GLFWwindow *window) {
    VkSurfaceKHR surface;
    vk_assert(glfwCreateWindowSurface(instance.get(), window, nullptr, &surface), "Failed to create window surface.");
    return unique_ptr_of<VkSurfaceKHR>(surface, [instance](VkSurfaceKHR surface) {
        debug_println("delete surface");
        vkDestroySurfaceKHR(instance.get(), surface, nullptr);
    });
}

shared_ptr_of<VkDevice> GraphicsManager::make_device(VkPhysicalDevice phys_device,
                                                     std::span<VkDeviceQueueCreateInfo const> queue_infos,
                                                     std::span<char const *const> extension_names) {
    VkPhysicalDeviceFeatures features;
    vkGetPhysicalDeviceFeatures(phys_device, &features);
    VkDeviceCreateInfo info{
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .queueCreateInfoCount = static_cast<uint32_t>(queue_infos.size()),
        .pQueueCreateInfos = queue_infos.data(),
        .enabledExtensionCount = static_cast<std::uint32_t>(extension_names.size()),
        .ppEnabledExtensionNames = extension_names.data(),
        .pEnabledFeatures = &features,
    };
    VkDevice device;
    vk_assert(vkCreateDevice(phys_device, &info, nullptr, &device), "Failed to create a device.");
    return shared_ptr_of<VkDevice>(device, [](VkDevice device) {
        debug_println("delete device");
        vkDestroyDevice(device, nullptr);
    });
}

unique_ptr_of<VkRenderPass> GraphicsManager::make_render_pass(shared_ptr_of<VkDevice> device, VkRenderPassCreateInfo const &info) {
    VkRenderPass render_pass;
    vk_assert(vkCreateRenderPass(device.get(), &info, nullptr, &render_pass), "Failed to create a render pass.");
    return unique_ptr_of<VkRenderPass>(render_pass, [device](VkRenderPass render_pass) {
        debug_println("delete render pass");
        vkDestroyRenderPass(device.get(), render_pass, nullptr);
    });
}

unique_ptr_of<VkSwapchainKHR> GraphicsManager::make_swapchain(shared_ptr_of<VkDevice> device, VkSwapchainCreateInfoKHR const &info) {
    VkSwapchainKHR swapchain;
    vk_assert(vkCreateSwapchainKHR(device.get(), &info, nullptr, &swapchain), "Failed to create a swapchain.");
    return unique_ptr_of<VkSwapchainKHR>(swapchain, [device](VkSwapchainKHR swapchain) {
        debug_println("delete swapchain");
        vkDestroySwapchainKHR(device.get(), swapchain, nullptr);
    });
}

shared_ptr_of<VkCommandPool>
GraphicsManager::make_command_pool(shared_ptr_of<VkDevice> device, VkCommandPoolCreateFlags create_flags, uint32_t qfm_index) {
    VkCommandPoolCreateInfo command_pool_info{
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = create_flags,
        .queueFamilyIndex = qfm_index,
    };
    VkCommandPool command_pool;
    vk_assert(vkCreateCommandPool(device.get(), &command_pool_info, nullptr, &command_pool), "Failed to create a command pool.");
    return shared_ptr_of<VkCommandPool>(command_pool, [device](VkCommandPool command_pool) {
        debug_println("delete command pool");
        vkDestroyCommandPool(device.get(), command_pool, nullptr);
    });
}

unique_ptr_of<VkBuffer>
GraphicsManager::make_buffer(shared_ptr_of<VkDevice> device, size_t size, VkBufferUsageFlags usage, VkSharingMode mode) {
    VkBufferCreateInfo info{
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = size,
        .usage = usage,
        .sharingMode = mode,
    };
    VkBuffer buffer;
    vk_assert(vkCreateBuffer(device.get(), &info, nullptr, &buffer), "Failed to create a buffer.");
    return unique_ptr_of<VkBuffer>(buffer, [device](VkBuffer buffer) {
        debug_println("delete buffer");
        vkDestroyBuffer(device.get(), buffer, nullptr);
    });
}

unique_ptr_of<VkCommandBuffer> GraphicsManager::make_command_buffer(shared_ptr_of<VkDevice> device,
                                                                    shared_ptr_of<VkCommandPool> command_pool) {
    VkCommandBufferAllocateInfo allocate_info{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = command_pool.get(),
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
    };
    VkCommandBuffer command_buffer;
    vk_assert(vkAllocateCommandBuffers(device.get(), &allocate_info, &command_buffer), "Failed to allocate a command buffer.");
    return unique_ptr_of<VkCommandBuffer>(command_buffer, [device, command_pool](VkCommandBuffer command_buffer) {
        debug_println("delete command buffer");
        vkFreeCommandBuffers(device.get(), command_pool.get(), 1, &command_buffer);
    });
}

unique_ptr_of<VkFramebuffer> GraphicsManager::make_framebuffer(shared_ptr_of<VkDevice> device,
                                                               std::span<VkImageView const> image_views,
                                                               VkRenderPass render_pass,
                                                               VkFormat format,
                                                               VkExtent2D extent) {
    VkFramebufferCreateInfo framebuffer_info{
        .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
        .renderPass = render_pass,
        .attachmentCount = static_cast<uint32_t>(image_views.size()),
        .pAttachments = image_views.data(),
        .width = extent.width,
        .height = extent.height,
        .layers = 1,
    };
    VkFramebuffer framebuffer;
    vk_assert(vkCreateFramebuffer(device.get(), &framebuffer_info, nullptr, &framebuffer), "Failed to create a framebuffer.");
    return unique_ptr_of<VkFramebuffer>(framebuffer, [device](VkFramebuffer framebuffer) {
        debug_println("delete framebuffer");
        vkDestroyFramebuffer(device.get(), framebuffer, nullptr);
    });
}

unique_ptr_of<VkImageView>
GraphicsManager::make_image_view(shared_ptr_of<VkDevice> device, VkImage image, VkFormat format, VkImageAspectFlags aspect) {
    VkImageViewCreateInfo image_view_info{
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = image,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = format,
        .components =
            VkComponentMapping{
                .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                .a = VK_COMPONENT_SWIZZLE_IDENTITY,
            },
        .subresourceRange =
            VkImageSubresourceRange{
                .aspectMask = aspect,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
    };
    VkImageView image_view;
    vk_assert(vkCreateImageView(device.get(), &image_view_info, nullptr, &image_view), "Failed to create an image view.");
    return unique_ptr_of<VkImageView>(image_view, [device](VkImageView image_view) {
        debug_println("delete image view");
        vkDestroyImageView(device.get(), image_view, nullptr);
    });
}

shared_ptr_of<VkDeviceMemory> GraphicsManager::make_device_memory(shared_ptr_of<VkDevice> device, size_t size, uint32_t type_index) {
    VkMemoryAllocateInfo info{
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = size,
        .memoryTypeIndex = type_index,
    };
    VkDeviceMemory memory;
    vk_assert(vkAllocateMemory(device.get(), &info, nullptr, &memory), "Failed to allocate a device memory.");
    return shared_ptr_of<VkDeviceMemory>(memory, [device](VkDeviceMemory device_memory) {
        debug_println("delete device memory");
        vkFreeMemory(device.get(), device_memory, nullptr);
    });
}

unique_ptr_of<VkFence> GraphicsManager::make_fence(shared_ptr_of<VkDevice> device) {
    VkFenceCreateInfo fence_info{
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT,
    };
    VkFence fence;
    vk_assert(vkCreateFence(device.get(), &fence_info, nullptr, &fence), "Failed to create a fence.");
    return unique_ptr_of<VkFence>(fence, [device](VkFence fence) {
        debug_println("delete fence");
        vkDestroyFence(device.get(), fence, nullptr);
    });
}

unique_ptr_of<VkSemaphore> GraphicsManager::make_semaphore(shared_ptr_of<VkDevice> device) {
    VkSemaphoreCreateInfo semaphore_info{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
    };
    VkSemaphore semaphore;
    vk_assert(vkCreateSemaphore(device.get(), &semaphore_info, nullptr, &semaphore), "Failed to create a semaphore.");
    return unique_ptr_of<VkSemaphore>(semaphore, [device](VkSemaphore semaphore) {
        debug_println("delete semaphore");
        vkDestroySemaphore(device.get(), semaphore, nullptr);
    });
}

unique_ptr_of<VkPipelineLayout> GraphicsManager::make_pipeline_layout(shared_ptr_of<VkDevice> device,
                                                                      std::span<VkDescriptorSetLayout const> set_layouts,
                                                                      std::span<VkPushConstantRange const> push_constant_ranges) {
    VkPipelineLayoutCreateInfo layout_info{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = static_cast<uint32_t>(set_layouts.size()),
        .pSetLayouts = set_layouts.data(),
        .pushConstantRangeCount = static_cast<uint32_t>(push_constant_ranges.size()),
        .pPushConstantRanges = push_constant_ranges.data(),
    };
    VkPipelineLayout pipeline_layout;
    vk_assert(vkCreatePipelineLayout(device.get(), &layout_info, nullptr, &pipeline_layout), "Failed to create pipeline layout.");
    return unique_ptr_of<VkPipelineLayout>(pipeline_layout, [device](VkPipelineLayout pipeline_layout) {
        debug_println("delete pipeline layout");
        vkDestroyPipelineLayout(device.get(), pipeline_layout, nullptr);
    });
}

unique_ptr_of<VkPipeline> GraphicsManager::make_pipeline(shared_ptr_of<VkDevice> device,
                                                         VkGraphicsPipelineCreateInfo const &pipeline_info) {
    VkPipeline pipeline;
    vk_assert(vkCreateGraphicsPipelines(device.get(), nullptr, 1, &pipeline_info, nullptr, &pipeline), "Failed to create a pipeline.");
    return unique_ptr_of<VkPipeline>(pipeline, [device](VkPipeline pipeline) {
        debug_println("delete pipeline");
        vkDestroyPipeline(device.get(), pipeline, nullptr);
    });
}

unique_ptr_of<VkDescriptorSetLayout>
GraphicsManager::make_descriptor_set_layout(shared_ptr_of<VkDevice> device, std::span<VkDescriptorSetLayoutBinding const> bindings) {
    VkDescriptorSetLayoutCreateInfo info{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = static_cast<uint32_t>(bindings.size()),
        .pBindings = bindings.data(),
    };
    VkDescriptorSetLayout layout;
    vk_assert(vkCreateDescriptorSetLayout(device.get(), &info, nullptr, &layout), "Failed to create descriptor set layout.");
    return unique_ptr_of<VkDescriptorSetLayout>(layout, [device](VkDescriptorSetLayout layout) {
        debug_println("delete descriptor set layout");
        vkDestroyDescriptorSetLayout(device.get(), layout, nullptr);
    });
}

unique_ptr_of<VkDescriptorPool> GraphicsManager::make_descriptor_pool(shared_ptr_of<VkDevice> device,
                                                                      uint32_t max_sets_count,
                                                                      std::span<VkDescriptorPoolSize const> pool_sizes) {
    VkDescriptorPoolCreateInfo info{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .maxSets = max_sets_count,
        .poolSizeCount = static_cast<uint32_t>(pool_sizes.size()),
        .pPoolSizes = pool_sizes.data(),
    };
    VkDescriptorPool descriptor_pool;
    vk_assert(vkCreateDescriptorPool(device.get(), &info, nullptr, &descriptor_pool), "Failed to create a descriptor pool.");
    return unique_ptr_of<VkDescriptorPool>(descriptor_pool, [device](VkDescriptorPool descriptor_pool) {
        debug_println("delete descriptor pool");
        vkDestroyDescriptorPool(device.get(), descriptor_pool, nullptr);
    });
}

std::vector<VkDescriptorSet> GraphicsManager::allocate_descriptor_sets(shared_ptr_of<VkDevice> device,
                                                                       VkDescriptorPool pool,
                                                                       std::span<VkDescriptorSetLayout const> layouts) {
    VkDescriptorSetAllocateInfo info{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = pool,
        .descriptorSetCount = static_cast<uint32_t>(layouts.size()),
        .pSetLayouts = layouts.data(),
    };
    std::vector<VkDescriptorSet> descriptor_sets(layouts.size());
    vk_assert(vkAllocateDescriptorSets(device.get(), &info, descriptor_sets.data()), "Failed to allocate descriptor sets.");
    return descriptor_sets;
}

unique_ptr_of<VkImage> GraphicsManager::make_image(shared_ptr_of<VkDevice> device, VkImageCreateInfo const &info) {
    VkImage image;
    vk_assert(vkCreateImage(device.get(), &info, nullptr, &image), "Failed to create image.");
    return unique_ptr_of<VkImage>(image, [device](VkImage image) {
        debug_println("delete image");
        vkDestroyImage(device.get(), image, nullptr);
    });
}

unique_ptr_of<VkSampler> GraphicsManager::make_sampler(shared_ptr_of<VkDevice> device, float anisotropy) {
    VkSamplerCreateInfo info{
        .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .magFilter = VK_FILTER_LINEAR,
        .minFilter = VK_FILTER_LINEAR,
        .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
        .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .mipLodBias = 0.0f,
        .anisotropyEnable = static_cast<VkBool32>(anisotropy != 1.0f),
        .maxAnisotropy = anisotropy,
        .compareEnable = VK_FALSE,
        .compareOp = VK_COMPARE_OP_ALWAYS,
        .minLod = 0.0f,
        .maxLod = 0.0f,
        .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
        .unnormalizedCoordinates = VK_FALSE,
    };
    VkSampler sampler;
    vk_assert(vkCreateSampler(device.get(), &info, nullptr, &sampler), "Failed to create sampler.");
    return unique_ptr_of<VkSampler>(sampler, [device](VkSampler sampler) {
        debug_println("delete sampler");
        vkDestroySampler(device.get(), sampler, nullptr);
    });
}
