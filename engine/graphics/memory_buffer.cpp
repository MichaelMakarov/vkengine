#include "memory_buffer.hpp"

#include "graphics_error.hpp"
#include "graphics_manager.hpp"

MemoryBuffer::MemoryBuffer(shared_ptr_of<VkDevice> device,
                           std::shared_ptr<AllocatorInterface> allocator,
                           VkMemoryAllocateFlags flags,
                           VkDeviceSize size,
                           VkBufferUsageFlags usage,
                           VkSharingMode mode)
    : device_{device.get()}
    , buffer_{GraphicsManager::make_buffer(device, size, usage, mode)}
    , allocator_{allocator} {
    VkMemoryRequirements requirements;
    vkGetBufferMemoryRequirements(device_, buffer_.get(), &requirements);
    block_ = allocator_->allocate(requirements, flags);
    alignment_ = requirements.alignment;
    VkDeviceMemory memory = block_.get_memory().get();
    VkDeviceSize offset = block_.get_offset(alignment_);
    vk_assert(vkBindBufferMemory(device_, buffer_.get(), memory, offset),
              "Failed to bind buffer={} to memory={} with offset={}.",
              reinterpret_cast<uintptr_t>(buffer_.get()),
              reinterpret_cast<uintptr_t>(memory),
              offset);
}

MemoryBuffer::~MemoryBuffer() {
    if (allocator_) {
        allocator_->deallocate(block_);
    }
}

void MemoryBuffer::fill(void const *data, size_t size) {
    VkDeviceMemory memory = block_.get_memory().get();
    VkDeviceSize offset = block_.get_offset(alignment_);
    VkDeviceSize bufsize = block_.get_size(alignment_);
    void *ptr;
    vk_assert(vkMapMemory(device_, memory, offset, bufsize, 0, &ptr),
              "Failed to map memory={}: offset={}, size={}.",
              reinterpret_cast<uintptr_t>(memory),
              offset,
              bufsize);
    std::memcpy(ptr, data, size);
    vkUnmapMemory(device_, memory);
}
