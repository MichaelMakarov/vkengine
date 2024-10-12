#pragma once

#include "graphics_types.hpp"

#include <vector>

class MemoryBuffer {
  public:
    struct Config {
        VkDeviceSize size;
        VkBufferUsageFlags usage;
        VkSharingMode mode = VK_SHARING_MODE_EXCLUSIVE;
    };

  private:
    shared_ptr_of<VkDevice> device_;
    shared_ptr_of<VkDeviceMemory> memory_;
    unique_ptr_of<VkBuffer> buffer_;
    VkMemoryRequirements requirements_;
    VkDeviceSize offset_;

    MemoryBuffer(shared_ptr_of<VkDevice> device, Config info);

    void bind(shared_ptr_of<VkDeviceMemory> memory);

  public:
    MemoryBuffer() = default;

    MemoryBuffer(shared_ptr_of<VkDevice> device, VkPhysicalDevice phys_device, Config info, VkMemoryPropertyFlags properties);

    MemoryBuffer(MemoryBuffer &&) noexcept = default;

    MemoryBuffer &operator=(MemoryBuffer &&) noexcept = default;

    static std::vector<MemoryBuffer> make_buffers(shared_ptr_of<VkDevice> device,
                                                  VkPhysicalDevice phys_device,
                                                  std::vector<Config> const &configs,
                                                  VkMemoryPropertyFlags properties);

    VkBuffer get_buffer() const {
        return buffer_.get();
    }

    VkDeviceSize get_size() const {
        return requirements_.size;
    }

    void fill(void const *data, size_t size);
};
