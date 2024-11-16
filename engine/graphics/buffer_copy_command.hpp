#pragma once

#include "command_interface.hpp"

class BufferCopyCommand : public CommandInterface {
    VkBuffer src_buffer_;
    VkBuffer dst_buffer_;
    VkDeviceSize size_;
    VkDeviceSize src_offset_ = 0;
    VkDeviceSize dst_offset_ = 0;

  public:
    BufferCopyCommand(VkBuffer src_buffer, VkDeviceSize src_offset, VkBuffer dst_buffer, VkDeviceSize dst_offset, VkDeviceSize size);

    BufferCopyCommand(VkBuffer src_buffer, VkBuffer dst_buffer, VkDeviceSize size);

    void execute(VkCommandBuffer command_buffer) override;
};