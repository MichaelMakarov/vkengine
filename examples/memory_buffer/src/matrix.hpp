#pragma once

#include "graphics/memory_buffer.hpp"

#include <glm/glm.hpp>
#include <vulkan/vulkan.hpp>

#include <chrono>

class Matrix {
    shared_ptr_of<VkDevice> device_;
    VkPhysicalDevice phys_device_;
    std::vector<MemoryBuffer> buffers_;
    unique_ptr_of<VkDescriptorSetLayout> set_layout_;
    unique_ptr_of<VkDescriptorPool> descriptor_pool_;
    std::vector<VkDescriptorSet> descriptor_sets_;
    VkExtent2D extent_;
    std::chrono::steady_clock::time_point const tn_;

  public:
    Matrix(shared_ptr_of<VkDevice> device, VkPhysicalDevice phys_device);

    void setup_buffers(size_t buffers_count, VkExtent2D extent);

    void update_content(size_t index);

    void setup_command_buffer(VkPipelineLayout layout, VkCommandBuffer command_buffer, size_t index);

    VkDescriptorSetLayout get_layout() const {
        return set_layout_.get();
    }
};