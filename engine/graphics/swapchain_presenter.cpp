#include "swapchain_presenter.hpp"

#include "graphics_error.hpp"
#include "graphics_manager.hpp"

namespace {

    uint64_t constexpr timeout = std::numeric_limits<uint64_t>::max();

}

SwapchainPresenter::SwapchainPresenter(shared_ptr_of<VkDevice> device, uint32_t graphics_qfm, uint32_t present_qfm)
    : device_{device}
    , sync_fence_{GraphicsManager::make_fence(device)} {
    vkGetDeviceQueue(device.get(), graphics_qfm, 0, &graphics_queue_);
    vkGetDeviceQueue(device.get(), present_qfm, 0, &present_queue_);
}

void SwapchainPresenter::submit_and_present(VkSwapchainKHR swapchain, ImageContext const &image_context) {
    constexpr uint32_t count = 1;
    VkPipelineStageFlags stage_flags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkSemaphore submit_semaphore = image_context.get_submit_semaphore();
    VkSemaphore present_semaphore = image_context.get_present_semaphore();
    VkCommandBuffer command_buffer = image_context.get_command_buffer();
    VkFence sync_fence = sync_fence_.get();

    vk_assert(vkWaitForFences(device_.get(), 1, &sync_fence, VK_TRUE, timeout), "Failed to wait for fences.");
    vk_assert(vkResetFences(device_.get(), 1, &sync_fence), "Failed to reset the fences.");

    uint32_t image_index;
    vk_assert(vkAcquireNextImageKHR(device_.get(), swapchain, timeout, submit_semaphore, nullptr, &image_index),
              "Failed to acquire next image.");

    if (frame_callback_) {
        frame_callback_(image_index);
    }

    VkSubmitInfo submit_info{
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = count,
        .pWaitSemaphores = &submit_semaphore,
        .pWaitDstStageMask = &stage_flags,
        .commandBufferCount = count,
        .pCommandBuffers = &command_buffer,
        .signalSemaphoreCount = count,
        .pSignalSemaphores = &present_semaphore,
    };
    vk_assert(vkQueueSubmit(graphics_queue_, 1, &submit_info, sync_fence), "Failed to submit the queue.");

    VkPresentInfoKHR present_info{
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = count,
        .pWaitSemaphores = &present_semaphore,
        .swapchainCount = count,
        .pSwapchains = &swapchain,
        .pImageIndices = &image_index,
        .pResults = nullptr,
    };
    vk_assert(vkQueuePresentKHR(present_queue_, &present_info), "Failed to present the queue.");
}
