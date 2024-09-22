#pragma once

#include "graphics_types.hpp"

struct queue_family {
    VkQueueFamilyProperties properties;
    uint32_t index;
};