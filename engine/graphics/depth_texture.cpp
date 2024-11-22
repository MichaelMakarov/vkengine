#include "depth_texture.hpp"

#include "graphics_error.hpp"
#include "graphics_manager.hpp"
#include "image_transition_command.hpp"
#include "memory_barrier.hpp"

namespace {

    bool has_stencil_component(VkFormat format) {
        return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
    }

} // namespace

DepthTexture::DepthTexture(shared_ptr_of<VkDevice> device,
                           std::shared_ptr<AllocatorInterface> allocator,
                           VkImageTiling tiling,
                           VkFormat format)
    : device_{device}
    , allocator_{allocator}
    , tiling_{tiling}
    , format_{format} {
}

DepthTexture::~DepthTexture() {
    if (block_) {
        allocator_->deallocate(block_);
    }
}

void DepthTexture::update_extent(VkExtent2D const &extent, Commander &transition_commander) {
    image_view_.release();
    if (block_) {
        allocator_->deallocate(block_);
    }
    VkImageCreateInfo info{
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = format_,
        .extent =
            VkExtent3D{
                .width = extent.width,
                .height = extent.height,
                .depth = 1,
            },
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = tiling_,
        .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    };
    image_ = GraphicsManager::make_image(device_, info);
    VkMemoryRequirements requirements;
    vkGetImageMemoryRequirements(device_.get(), image_.get(), &requirements);
    block_ = allocator_->allocate(requirements, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    VkDeviceMemory memory = block_.get_memory().get();
    VkDeviceSize offset = block_.get_offset(requirements.alignment);
    vk_assert(vkBindImageMemory(device_.get(), image_.get(), memory, offset),
              "Failed to bind depth image={} to memory={} with offset={}.",
              reinterpret_cast<uintptr_t>(image_.get()),
              reinterpret_cast<uintptr_t>(memory),
              offset);
    image_view_ = GraphicsManager::make_image_view(device_, image_.get(), format_, VK_IMAGE_ASPECT_DEPTH_BIT);
    VkImageAspectFlags image_aspect = VK_IMAGE_ASPECT_DEPTH_BIT;
    if (has_stencil_component(format_)) {
        image_aspect |= VK_IMAGE_ASPECT_STENCIL_BIT;
    }
    transition_commander.add_command(std::make_unique<ImageTransitionCommand>(
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
        MemoryBarrier::make_image_barrier(
            image_.get(),
            image_aspect,
            MemoryBarrier::ImageInfo{
                .access_mask = 0,
                .layout = VK_IMAGE_LAYOUT_UNDEFINED,
            },
            MemoryBarrier::ImageInfo{
                .access_mask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
                .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
            })));
    transition_commander.execute();
}
