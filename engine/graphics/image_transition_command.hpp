#pragma once

#include "memory_barrier_command.hpp"

class ImageTransitionCommand : public MemoryBarrierCommand {
  public:
    ImageTransitionCommand(VkPipelineStageFlags src_stage, VkPipelineStageFlags dst_stage, VkImageMemoryBarrier const &barrier);
};