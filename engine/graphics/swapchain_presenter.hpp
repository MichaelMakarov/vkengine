#pragma once

#include "graphics_types.hpp"

#include "image_context.hpp"

class SwapchainPresenter {
  public:
    using update_frame_t = std::function<void(size_t frame_index)>;

  private:
    shared_ptr_of<VkDevice> device_;
    unique_ptr_of<VkFence> sync_fence_;
    VkQueue graphics_queue_;
    VkQueue present_queue_;
    update_frame_t frame_callback_;

  public:
    SwapchainPresenter(shared_ptr_of<VkDevice> device, VkQueue graphics_queue, VkQueue present_queue);

    void submit_and_present(VkSwapchainKHR swapchain, ImageContext const &image_context);

    void set_update_frame_callback(update_frame_t const &callback) {
        frame_callback_ = callback;
    }
};