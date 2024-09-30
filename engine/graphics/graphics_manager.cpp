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
            debug_println("VK debug messenger: {}", callback_data->pMessage);
        }
        return VK_FALSE;
    }

    std::vector<uint8_t> read_shader(shared_ptr_of<VkDevice> device, std::string_view filename) {
        std::ifstream fin{filename.data(), std::ios::ate | std::ios::binary};
        if (!fin.is_open()) {
            raise_error("Failed to open shader file {}", filename);
        }
        std::vector<uint8_t> buffer(static_cast<size_t>(fin.tellg()));
        fin.seekg(0);
        fin.read(reinterpret_cast<char *>(buffer.data()), buffer.size());
        fin.close();
        return buffer;
    }

} // namespace

shared_ptr_of<VkInstance> graphics_manager::make_instance(char const *app_name,
                                                          std::vector<char const *> const &extensions,
                                                          std::vector<char const *> const &layers) {
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
        .enabledLayerCount = static_cast<std::uint32_t>(layers.size()),
        .ppEnabledLayerNames = layers.data(),
        .enabledExtensionCount = static_cast<std::uint32_t>(extensions.size()),
        .ppEnabledExtensionNames = extensions.data(),
    };
    VkInstance instance;
    vk_assert(vkCreateInstance(&instance_info, nullptr, &instance), "Failed to create instance.");
    return shared_ptr_of<VkInstance>(instance, [](VkInstance instance) {
        debug_println("delete instance");
        vkDestroyInstance(instance, nullptr);
    });
}

unique_ptr_of<VkDebugUtilsMessengerEXT> graphics_manager::make_debug_messenger(shared_ptr_of<VkInstance> instance) {
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

unique_ptr_of<VkSurfaceKHR> graphics_manager::make_surface(shared_ptr_of<VkInstance> instance, GLFWwindow *window) {
    VkSurfaceKHR surface;
    vk_assert(glfwCreateWindowSurface(instance.get(), window, nullptr, &surface), "Failed to create window surface.");
    return unique_ptr_of<VkSurfaceKHR>(surface, [instance](VkSurfaceKHR surface) {
        debug_println("delete surface");
        vkDestroySurfaceKHR(instance.get(), surface, nullptr);
    });
}

shared_ptr_of<VkDevice> graphics_manager::make_device(VkPhysicalDevice phys_device,
                                                      std::vector<VkDeviceQueueCreateInfo> const &queue_infos,
                                                      std::vector<char const *> const &extension_names) {
    VkPhysicalDeviceFeatures features{};
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

unique_ptr_of<VkRenderPass> graphics_manager::make_render_pass(shared_ptr_of<VkDevice> device, VkFormat format) {
    VkAttachmentDescription attachment{
        .format = format,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
    };
    VkAttachmentReference color_attachment{
        .attachment = 0,
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    };
    VkSubpassDescription subpass{
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .colorAttachmentCount = 1,
        .pColorAttachments = &color_attachment,
    };
    VkSubpassDependency dependency{
        .srcSubpass = VK_SUBPASS_EXTERNAL,
        .dstSubpass = 0,
        .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .srcAccessMask = 0,
        .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
    };
    VkRenderPassCreateInfo info{
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .attachmentCount = 1,
        .pAttachments = &attachment,
        .subpassCount = 1,
        .pSubpasses = &subpass,
        .dependencyCount = 1,
        .pDependencies = &dependency,
    };

    VkRenderPass render_pass;
    vk_assert(vkCreateRenderPass(device.get(), &info, nullptr, &render_pass), "Failed to create a render pass.");
    return unique_ptr_of<VkRenderPass>(render_pass, [device](VkRenderPass render_pass) {
        debug_println("delete render pass");
        vkDestroyRenderPass(device.get(), render_pass, nullptr);
    });
}

unique_ptr_of<VkSwapchainKHR> graphics_manager::make_swapchain(shared_ptr_of<VkDevice> device,
                                                               VkSwapchainCreateInfoKHR const &swapchain_info) {
    VkSwapchainKHR swapchain;
    vk_assert(vkCreateSwapchainKHR(device.get(), &swapchain_info, nullptr, &swapchain), "Failed to create a swapchain.");
    return unique_ptr_of<VkSwapchainKHR>(swapchain, [device](VkSwapchainKHR swapchain) {
        debug_println("delete swapchain");
        vkDestroySwapchainKHR(device.get(), swapchain, nullptr);
    });
}

shared_ptr_of<VkCommandPool>
graphics_manager::make_command_pool(shared_ptr_of<VkDevice> device, VkCommandPoolCreateFlags create_flags, uint32_t qfm_index) {
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
graphics_manager::make_buffer(shared_ptr_of<VkDevice> device, size_t size, VkBufferUsageFlags usage, VkSharingMode mode) {
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

unique_ptr_of<VkCommandBuffer> graphics_manager::make_command_buffer(shared_ptr_of<VkDevice> device,
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

unique_ptr_of<VkFramebuffer> graphics_manager::make_framebuffer(shared_ptr_of<VkDevice> device,
                                                                VkImageView image_view,
                                                                VkRenderPass render_pass,
                                                                VkFormat format,
                                                                VkExtent2D extent) {
    VkFramebufferCreateInfo framebuffer_info{
        .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
        .renderPass = render_pass,
        .attachmentCount = 1,
        .pAttachments = &image_view,
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

unique_ptr_of<VkImageView> graphics_manager::make_image_view(shared_ptr_of<VkDevice> device, VkImage image, VkFormat format) {
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
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
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

unique_ptr_of<VkDeviceMemory>
graphics_manager::make_device_memory(shared_ptr_of<VkDevice> device, size_t size, uint32_t type_index) {
    VkMemoryAllocateInfo info{
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = size,
        .memoryTypeIndex = type_index,
    };
    VkDeviceMemory memory;
    vk_assert(vkAllocateMemory(device.get(), &info, nullptr, &memory), "Failed to allocate a device memory.");
    return unique_ptr_of<VkDeviceMemory>(memory, [device](VkDeviceMemory device_memory) {
        debug_println("delete device memory");
        vkFreeMemory(device.get(), device_memory, nullptr);
    });
}

unique_ptr_of<VkFence> graphics_manager::make_fence(shared_ptr_of<VkDevice> device) {
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

unique_ptr_of<VkSemaphore> graphics_manager::make_semaphore(shared_ptr_of<VkDevice> device) {
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

unique_ptr_of<VkPipelineLayout> graphics_manager::make_pipeline_layout(shared_ptr_of<VkDevice> device,
                                                                       std::vector<VkDescriptorSetLayout> const &set_layouts,
                                                                       std::vector<VkPushConstantRange> const &push_constant_ranges) {
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

unique_ptr_of<VkShaderModule> graphics_manager::make_shader_module(shared_ptr_of<VkDevice> device, std::string_view filename) {
    auto buffer = read_shader(device, filename);
    VkShaderModuleCreateInfo info{
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = buffer.size(),
        .pCode = reinterpret_cast<uint32_t const *>(buffer.data()),
    };
    VkShaderModule shader_module;
    vk_assert(vkCreateShaderModule(device.get(), &info, nullptr, &shader_module), "Failed to create a shader module.");
    return unique_ptr_of<VkShaderModule>(shader_module, [device](VkShaderModule shader_module) {
        debug_println("delete shader module");
        vkDestroyShaderModule(device.get(), shader_module, nullptr);
    });
}

VkPipelineShaderStageCreateInfo graphics_manager::make_shader_stage(VkShaderModule shader_module, VkShaderStageFlagBits stage) {
    return VkPipelineShaderStageCreateInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = stage,
        .module = shader_module,
        .pName = "main",
    };
}

unique_ptr_of<VkPipeline> graphics_manager::make_pipeline(shared_ptr_of<VkDevice> device,
                                                          VkGraphicsPipelineCreateInfo const &pipeline_info) {
    VkPipeline pipeline;
    vk_assert(vkCreateGraphicsPipelines(device.get(), nullptr, 1, &pipeline_info, nullptr, &pipeline), "Failed to create a pipeline.");
    return unique_ptr_of<VkPipeline>(pipeline, [device](VkPipeline pipeline) {
        debug_println("delete pipeline");
        vkDestroyPipeline(device.get(), pipeline, nullptr);
    });
}


