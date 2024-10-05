#pragma once

#include "graphics_types.hpp"
#include "memory_buffer.hpp"

class BufferManager {
    shared_ptr_of<VkDevice> device_;
    shared_ptr_of<VkCommandPool> command_pool_;
    VkQueue queue_;

  public:
    BufferManager(shared_ptr_of<VkDevice> device, uint32_t transfer_qfm);

    void copy_buffers(MemoryBuffer const *src_buffers, MemoryBuffer const *dst_buffers, size_t buffers_count);
};