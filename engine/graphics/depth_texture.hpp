#pragma once

#include "allocator_interface.hpp"
#include "commander.hpp"

class DepthTexture {
    shared_ptr_of<VkDevice> device_;
    unique_ptr_of<VkImage> image_;
    unique_ptr_of<VkImageView> image_view_;
    std::shared_ptr<AllocatorInterface> allocator_;
    VkImageTiling tiling_;
    VkFormat format_;
    MemoryBlock block_;

  public:
    DepthTexture(shared_ptr_of<VkDevice> device, std::shared_ptr<AllocatorInterface> allocator, VkImageTiling tiling, VkFormat format);

    ~DepthTexture();

    VkImage get_image() const {
        return image_.get();
    }

    VkImageView get_image_view() const {
        return image_view_.get();
    }

    void update_extent(VkExtent2D const &extent, Commander &transition_commander);
};