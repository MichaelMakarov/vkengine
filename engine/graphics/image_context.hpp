#pragma once

#include "graphics_types.hpp"

class ImageContext {
    unique_ptr_of<VkImageView> image_view_;
    unique_ptr_of<VkFramebuffer> framebuffer_;
    unique_ptr_of<VkSemaphore> submit_semaphore_;
    unique_ptr_of<VkSemaphore> present_semaphore_;
    unique_ptr_of<VkCommandBuffer> command_buffer_;

  public:
    VkImageView get_image_view() const {
        return image_view_.get();
    }
    
    void set_image_view(unique_ptr_of<VkImageView> image_view) {
        image_view_.swap(image_view);
    }

    VkFramebuffer get_framebuffer() const {
        return framebuffer_.get();
    }

    void set_framebuffer(unique_ptr_of<VkFramebuffer> framebuffer) {
        framebuffer_.swap(framebuffer);
    }

    VkSemaphore get_submit_semaphore() const {
        return submit_semaphore_.get();
    }

    void set_submit_semaphore(unique_ptr_of<VkSemaphore> semaphore) {
        submit_semaphore_.swap(semaphore);
    }

    VkSemaphore get_present_semaphore() const {
        return present_semaphore_.get();
    }

    void set_present_semaphore(unique_ptr_of<VkSemaphore> semaphore) {
        present_semaphore_.swap(semaphore);
    }

    VkCommandBuffer get_command_buffer() const {
        return command_buffer_.get();
    }

    void set_command_buffer(unique_ptr_of<VkCommandBuffer> command_buffer) {
        command_buffer_.swap(command_buffer);
    }
};