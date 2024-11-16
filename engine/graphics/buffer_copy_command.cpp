#include "buffer_copy_command.hpp"

BufferCopyCommand::BufferCopyCommand(VkBuffer src_buffer, VkDeviceSize src_offset, VkBuffer dst_buffer, VkDeviceSize dst_offset, VkDeviceSize size)
    : src_buffer_{src_buffer}
    , src_offset_{src_offset}
    , dst_buffer_{dst_buffer}
    , dst_offset_{dst_offset}
    , size_{size} {
}

BufferCopyCommand::BufferCopyCommand(VkBuffer src_buffer, VkBuffer dst_buffer, VkDeviceSize size)
    : BufferCopyCommand(src_buffer, 0, dst_buffer, 0, size) {
}

void BufferCopyCommand::execute(VkCommandBuffer command_buffer) {
    VkBufferCopy region{
        .srcOffset = src_offset_,
        .dstOffset = dst_offset_,
        .size = size_,
    };
    vkCmdCopyBuffer(command_buffer, src_buffer_, dst_buffer_, 1, &region);
}