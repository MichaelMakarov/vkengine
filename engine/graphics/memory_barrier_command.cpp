#include "memory_barrier_command.hpp"

void MemoryBarrierCommand::execute(VkCommandBuffer command_buffer) {
    if (memory_barriers_.empty() && buffer_barriers_.empty() && image_barriers_.empty()) {
        return;
    }
    vkCmdPipelineBarrier(command_buffer,
                         src_stage_,
                         dst_stage_,
                         0,
                         static_cast<uint32_t>(memory_barriers_.size()),
                         memory_barriers_.data(),
                         static_cast<uint32_t>(buffer_barriers_.size()),
                         buffer_barriers_.data(),
                         static_cast<uint32_t>(image_barriers_.size()),
                         image_barriers_.data());
}