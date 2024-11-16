#include "memory_barrier.hpp"

VkImageMemoryBarrier MemoryBarrier::make_image_barrier(VkImage image, ImageInfo const &src_info, ImageInfo const &dst_info) {
    return VkImageMemoryBarrier{
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .srcAccessMask = src_info.access_mask,
        .dstAccessMask = dst_info.access_mask,
        .oldLayout = src_info.layout,
        .newLayout = dst_info.layout,
        .srcQueueFamilyIndex = src_info.qfm_index,
        .dstQueueFamilyIndex = dst_info.qfm_index,
        .image = image,
        .subresourceRange =
            VkImageSubresourceRange{
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
    };
}
