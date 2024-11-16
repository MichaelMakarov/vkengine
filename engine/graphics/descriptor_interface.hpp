#pragma once

#include "graphics_types.hpp"

class DescriptorInterface {
  public:
    virtual ~DescriptorInterface() = default;

    virtual VkDescriptorSetLayoutBinding get_binding() const = 0;

    virtual VkWriteDescriptorSet get_write(VkDescriptorSet descriptor_set, size_t image_index) const = 0;
};