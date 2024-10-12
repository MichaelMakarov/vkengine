#pragma once

#include "graphics_types.hpp"

#include <GLFW/glfw3.h>

#include <string_view>
#include <vector>

class GraphicsManager {
  public:
    static shared_ptr_of<VkInstance>
    make_instance(char const *app_name, std::vector<char const *> const &extensions, std::vector<char const *> const &layers);

    static unique_ptr_of<VkDebugUtilsMessengerEXT> make_debug_messenger(shared_ptr_of<VkInstance> instance);

    static unique_ptr_of<VkSurfaceKHR> make_surface(shared_ptr_of<VkInstance> instance, GLFWwindow *window);

    static shared_ptr_of<VkDevice> make_device(VkPhysicalDevice phys_device,
                                               std::vector<VkDeviceQueueCreateInfo> const &queue_infos,
                                               std::vector<char const *> const &extension_names);

    static unique_ptr_of<VkRenderPass> make_render_pass(shared_ptr_of<VkDevice> device, VkFormat format);

    static unique_ptr_of<VkSwapchainKHR> make_swapchain(shared_ptr_of<VkDevice> device, VkSwapchainCreateInfoKHR const &swapchain_info);

    static shared_ptr_of<VkCommandPool>
    make_command_pool(shared_ptr_of<VkDevice> device, VkCommandPoolCreateFlags create_flags, uint32_t qfm_index);

    static unique_ptr_of<VkBuffer> make_buffer(shared_ptr_of<VkDevice> device, size_t size, VkBufferUsageFlags usage, VkSharingMode mode);

    static unique_ptr_of<VkCommandBuffer> make_command_buffer(shared_ptr_of<VkDevice> device, shared_ptr_of<VkCommandPool> command_pool);

    static unique_ptr_of<VkFramebuffer>
    make_framebuffer(shared_ptr_of<VkDevice> device, VkImageView image_view, VkRenderPass render_pass, VkFormat format, VkExtent2D extent);

    static unique_ptr_of<VkImageView> make_image_view(shared_ptr_of<VkDevice> device, VkImage image, VkFormat format);

    static unique_ptr_of<VkDeviceMemory> make_device_memory(shared_ptr_of<VkDevice> device, size_t size, uint32_t type_index);

    static unique_ptr_of<VkFence> make_fence(shared_ptr_of<VkDevice> device);

    static unique_ptr_of<VkSemaphore> make_semaphore(shared_ptr_of<VkDevice> device);

    static unique_ptr_of<VkPipelineLayout> make_pipeline_layout(shared_ptr_of<VkDevice> device,
                                                                std::vector<VkDescriptorSetLayout> const &set_layouts,
                                                                std::vector<VkPushConstantRange> const &push_constant_ranges);

    static unique_ptr_of<VkShaderModule> make_shader_module(shared_ptr_of<VkDevice> device, std::string_view filename);

    static VkPipelineShaderStageCreateInfo make_shader_stage(VkShaderModule shader_module, VkShaderStageFlagBits stage);

    static unique_ptr_of<VkPipeline> make_pipeline(shared_ptr_of<VkDevice> device, VkGraphicsPipelineCreateInfo const &pipeline_info);

    static unique_ptr_of<VkDescriptorSetLayout> make_descriptor_set_layout(shared_ptr_of<VkDevice> device,
                                                                           std::vector<VkDescriptorSetLayoutBinding> const &bindings);

    static unique_ptr_of<VkDescriptorPool> make_descriptor_pool(shared_ptr_of<VkDevice> device,
                                                                std::vector<VkDescriptorPoolSize> const &pool_sizes);

    static std::vector<VkDescriptorSet>
    allocate_descriptor_sets(shared_ptr_of<VkDevice> device, VkDescriptorPool pool, std::vector<VkDescriptorSetLayout> const &layouts);
};
