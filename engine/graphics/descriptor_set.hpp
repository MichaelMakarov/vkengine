#pragma once

#include "descriptor_interface.hpp"

#include <vector>

class DescriptorSet {
    struct DescriptorInfo {
        std::shared_ptr<DescriptorInterface> descriptor;
        VkDescriptorType type;
    };

    shared_ptr_of<VkDevice> device_;
    unique_ptr_of<VkDescriptorSetLayout> set_layout_;
    unique_ptr_of<VkDescriptorPool> descriptor_pool_;
    std::vector<VkDescriptorSet> descriptor_sets_;
    std::vector<DescriptorInfo> descriptors_;
    

  public:
    DescriptorSet(shared_ptr_of<VkDevice> device, std::vector<std::shared_ptr<DescriptorInterface>> const &descriptors);

    void set_swapchain_images_count(uint32_t images_count);

    VkDescriptorSet get_descriptor_set(size_t image_index) const {
        return descriptor_sets_[image_index];
    }

    VkDescriptorSetLayout get_layout() const {
        return set_layout_.get();
    }
};