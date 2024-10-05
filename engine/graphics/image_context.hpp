#pragma once

#include "graphics_types.hpp"

class ImageContext {
  public:
    struct context_info {
        VkImage image;
        VkFormat format;
        VkExtent2D extent;
        VkRenderPass render_pass;
    };

  private:
    unique_ptr_of<VkImageView> image_view_;
    unique_ptr_of<VkFramebuffer> framebuffer_;
    unique_ptr_of<VkSemaphore> submit_semaphore_;
    unique_ptr_of<VkSemaphore> present_semaphore_;
    unique_ptr_of<VkCommandBuffer> command_buffer_;

  public:
    ImageContext() = default;

    ImageContext(shared_ptr_of<VkDevice> device, context_info const &info, shared_ptr_of<VkCommandPool> cmd_pool);

    VkImageView get_image_view() const {
        return image_view_.get();
    }

    VkFramebuffer get_framebuffer() const {
        return framebuffer_.get();
    }

    VkSemaphore get_submit_semaphore() const {
        return submit_semaphore_.get();
    }

    VkSemaphore get_present_semaphore() const {
        return present_semaphore_.get();
    }

    VkCommandBuffer get_command_buffer() const {
        return command_buffer_.get();
    }
};