#include "image_copy_command.hpp"

#include "memory_barrier.hpp"
#include "memory_barrier_command.hpp"

ImageCopyCommand::ImageCopyCommand(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
    : buffer_{buffer}
    , image_{image}
    , width_{width}
    , height_{height} {
}

void ImageCopyCommand::execute(VkCommandBuffer command_buffer) {
    VkImageMemoryBarrier barrier = MemoryBarrier::make_image_barrier(image_,
                                                                     MemoryBarrier::ImageInfo{
                                                                         .access_mask = 0,
                                                                         .layout = VK_IMAGE_LAYOUT_UNDEFINED,
                                                                         .qfm_index = VK_QUEUE_FAMILY_IGNORED,
                                                                     },
                                                                     MemoryBarrier::ImageInfo{
                                                                         .access_mask = VK_ACCESS_TRANSFER_WRITE_BIT,
                                                                         .layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                                                         .qfm_index = VK_QUEUE_FAMILY_IGNORED,
                                                                     });
    vkCmdPipelineBarrier(command_buffer,
                         VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                         VK_PIPELINE_STAGE_TRANSFER_BIT,
                         0,
                         0,
                         nullptr,
                         0,
                         nullptr,
                         1,
                         &barrier);
    VkBufferImageCopy region{
        .bufferOffset = 0,
        .bufferRowLength = 0,
        .bufferImageHeight = 0,
        .imageSubresource =
            VkImageSubresourceLayers{
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .mipLevel = 0,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
        .imageOffset = VkOffset3D{.x = 0, .y = 0, .z = 0},
        .imageExtent = VkExtent3D{.width = width_, .height = height_, .depth = 1},
    };
    vkCmdCopyBufferToImage(command_buffer, buffer_, image_, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
    // barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    // barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    // vkCmdPipelineBarrier(command_buffer,
    //                      VK_PIPELINE_STAGE_TRANSFER_BIT,
    //                      VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
    //                      0,
    //                      0,
    //                      nullptr,
    //                      0,
    //                      nullptr,
    //                      1,
    //                      &barrier);
}
