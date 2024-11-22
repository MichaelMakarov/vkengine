#pragma once

#include "allocator_interface.hpp"

class ImageTexture {
    VkDevice device_;
    std::shared_ptr<AllocatorInterface> allocator_;
    MemoryBlock block_;
    unique_ptr_of<VkImage> image_;
    unique_ptr_of<VkImageView> image_view_;
    unique_ptr_of<VkSampler> sampler_;

  public:
    static constexpr VkFormat default_format = VK_FORMAT_R8G8B8A8_UNORM;

    ImageTexture(shared_ptr_of<VkDevice> device,
                 std::shared_ptr<AllocatorInterface> allocator,
                 VkMemoryAllocateFlags flags,
                 uint32_t width,
                 uint32_t height,
                 VkFormat format = VK_FORMAT_R8G8B8A8_UNORM);

    ImageTexture() = default;
    ImageTexture(ImageTexture &&) noexcept = default;

    ~ImageTexture();

    ImageTexture &operator=(ImageTexture &&) noexcept = default;

    VkImage get_image() const {
        return image_.get();
    }

    VkImageView get_image_view() const {
        return image_view_.get();
    }

    VkSampler get_sampler() const {
        return sampler_.get();
    }
};