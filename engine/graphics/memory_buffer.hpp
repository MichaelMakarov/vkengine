#pragma once

#include "graphics_types.hpp"

class memory_buffer {
    struct buffer_properties {
        VkDeviceSize offset;
        VkMemoryRequirements requirements;
    };

    shared_ptr_of<VkDevice> device_;
    unique_ptr_of<VkDeviceMemory> memory_;
    unique_ptr_of<VkBuffer> buffer_;
    VkMemoryRequirements requirements_;

  public:
    struct config {
        size_t size;
        VkBufferUsageFlags usage;
        VkSharingMode mode;
        VkMemoryPropertyFlags properties;
    };

    memory_buffer(shared_ptr_of<VkDevice> device, VkPhysicalDevice phys_device, config const &info);

    VkBuffer get_buffer() const {
        return buffer_.get();
    }

    void fill(void const *data, size_t size);
};