#pragma once

#include "graphics_types.hpp"

#include "swapchain_context.hpp"

class SwapchainPresenter {
    shared_ptr_of<VkDevice> device_;
    unique_ptr_of<VkFence> sync_fence_;
    VkQueue graphics_queue_;
    VkQueue present_queue_;


  public:
    SwapchainPresenter(shared_ptr_of<VkDevice> device, uint32_t graphics_qfm, uint32_t present_qfm);

    void submit_and_present(SwapchainContext &swapchain_ctx);
};