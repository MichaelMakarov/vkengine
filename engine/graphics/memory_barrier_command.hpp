#pragma once

#include "command_interface.hpp"

#include <vector>

class MemoryBarrierCommand : public CommandInterface {
    VkPipelineStageFlags src_stage_;
    VkPipelineStageFlags dst_stage_;
    std::vector<VkMemoryBarrier> memory_barriers_;
    std::vector<VkBufferMemoryBarrier> buffer_barriers_;
    std::vector<VkImageMemoryBarrier> image_barriers_;

  public:
    MemoryBarrierCommand(VkPipelineStageFlags src_stage, VkPipelineStageFlags dst_stage)
        : src_stage_{src_stage}
        , dst_stage_{dst_stage} {
    }

    void add_memory_barrier(VkMemoryBarrier const &barrier) {
        memory_barriers_.push_back(barrier);
    }

    void add_buffer_barrier(VkBufferMemoryBarrier const &barrier) {
        buffer_barriers_.push_back(barrier);
    }

    void add_image_barrier(VkImageMemoryBarrier const &barrier) {
        image_barriers_.push_back(barrier);
    }

    void execute(VkCommandBuffer command_buffer) override;
};