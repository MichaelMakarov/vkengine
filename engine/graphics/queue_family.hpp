#pragma once

#include "graphics_types.hpp"

struct QueueFamily {
    VkQueueFamilyProperties properties;
    uint32_t index;
};