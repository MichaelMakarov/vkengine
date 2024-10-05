#pragma once

#include "graphics_types.hpp"

class PipelineProvider {
  public:
    virtual ~PipelineProvider() = default;

    virtual void update_command_buffer(VkCommandBuffer command_buffer) = 0;

    virtual void set_update_rendering(std::function<void()> const &callback) = 0;
};