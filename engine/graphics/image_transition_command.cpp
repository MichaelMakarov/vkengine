#include "image_transition_command.hpp"

ImageTransitionCommand::ImageTransitionCommand(VkPipelineStageFlags src_stage,
                                               VkPipelineStageFlags dst_stage,
                                               VkImageMemoryBarrier const &barrier)
    : MemoryBarrierCommand(src_stage, dst_stage) {
    add_image_barrier(barrier);
}