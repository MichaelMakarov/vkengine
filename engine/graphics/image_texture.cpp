#include "image_texture.hpp"

#include "graphics_error.hpp"
#include "graphics_manager.hpp"

ImageTexture::ImageTexture(shared_ptr_of<VkDevice> device,
                           std::shared_ptr<AllocatorInterface> allocator,
                           VkMemoryAllocateFlags flags,
                           uint32_t width,
                           uint32_t height,
                           VkFormat format)
    : device_{device.get()}
    , allocator_{allocator} {
    VkImageCreateInfo info{
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = format,
        .extent =
            VkExtent3D{
                .width = width,
                .height = height,
                .depth = 1,
            },
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    };
    image_ = GraphicsManager::make_image(device, info);
    VkMemoryRequirements requirements;
    vkGetImageMemoryRequirements(device_, image_.get(), &requirements);
    block_ = allocator_->allocate(requirements, flags);
    VkDeviceMemory memory = block_.get_memory().get();
    VkDeviceSize offset = block_.get_offset(requirements.alignment);
    vk_assert(vkBindImageMemory(device_, image_.get(), memory, offset),
              "Failed to bind image={} to memory={} with offset={}.",
              reinterpret_cast<uintptr_t>(image_.get()),
              reinterpret_cast<uintptr_t>(memory),
              offset);
    image_view_ = GraphicsManager::make_image_view(device, image_.get(), format, VK_IMAGE_ASPECT_COLOR_BIT);
    sampler_ = GraphicsManager::make_sampler(device, 16);
}

ImageTexture::~ImageTexture() {
    if (allocator_) {
        allocator_->deallocate(block_);
    }
}

