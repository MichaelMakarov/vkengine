#pragma once

#include "graphics_types.hpp"

class MemoryBarrier {
  public:
    struct ImageInfo {
        VkAccessFlags access_mask = 0;
        VkImageLayout layout = VK_IMAGE_LAYOUT_UNDEFINED;
        uint32_t qfm_index = VK_QUEUE_FAMILY_IGNORED;
    };

    static VkImageMemoryBarrier make_image_barrier(VkImage image, ImageInfo const &src_info, ImageInfo const &dst_info);
};