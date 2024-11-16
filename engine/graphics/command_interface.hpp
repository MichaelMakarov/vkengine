#pragma once

#include "graphics/graphics_types.hpp"

class CommandInterface {
  public:
    virtual ~CommandInterface() = default;

    virtual void execute(VkCommandBuffer command_buffer) = 0;
};