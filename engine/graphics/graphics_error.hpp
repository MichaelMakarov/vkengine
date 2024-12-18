#pragma once

#include "graphics_types.hpp"

#include "utility/error.hpp"
#include "utility/log.hpp"

#define vk_assert(res, msg, ...)                                                                                                           \
    if (res != VK_SUCCESS)                                                                                                                 \
        raise_error(msg, __VA_ARGS__);