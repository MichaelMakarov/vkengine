#pragma once

#include "allocator_interface.hpp"

class MemoryBuffer {
    VkDevice device_ = nullptr;
    unique_ptr_of<VkBuffer> buffer_;
    std::shared_ptr<AllocatorInterface> allocator_;
    MemoryBlock block_;
    VkDeviceSize alignment_ = 1;

  public:
    MemoryBuffer(shared_ptr_of<VkDevice> device,
                 std::shared_ptr<AllocatorInterface> allocator,
                 VkMemoryAllocateFlags flags,
                 VkDeviceSize size,
                 VkBufferUsageFlags usage,
                 VkSharingMode mode = VK_SHARING_MODE_EXCLUSIVE);

    MemoryBuffer() = default;
    MemoryBuffer(MemoryBuffer &&) noexcept = default;

    ~MemoryBuffer();

    MemoryBuffer &operator=(MemoryBuffer &&) noexcept = default;

    VkBuffer get_buffer() const {
        return buffer_.get();
    }

    VkDeviceSize get_size() const {
        return block_.get_size(alignment_);
    }

    void fill(void const *data, size_t size);
};
