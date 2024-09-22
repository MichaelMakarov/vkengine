#pragma once

#include "graphics_types.hpp"

#include "swapchain_context.hpp"

class swapchain_presenter {
    shared_ptr_of<VkDevice> device_;
    unique_ptr_of<VkFence> sync_fence_;
    VkQueue graphics_queue_;
    VkQueue present_queue_;


  public:
    swapchain_presenter(shared_ptr_of<VkDevice> device, VkQueue graphics_queue, VkQueue present_queue);

    void submit_and_present(swapchain_context &swapchain_ctx);
};