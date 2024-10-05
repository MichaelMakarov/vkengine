#include "swapchain_presenter.hpp"

#include "graphics_manager.hpp"
#include "graphics_error.hpp"

#include <array>

namespace {

    uint64_t constexpr timeout = std::numeric_limits<uint64_t>::max();

}

SwapchainPresenter::SwapchainPresenter(shared_ptr_of<VkDevice> device, uint32_t graphics_qfm, uint32_t present_qfm)
    : device_{device}
    , sync_fence_{GraphicsManager::make_fence(device)} {
    vkGetDeviceQueue(device.get(), graphics_qfm, 0, &graphics_queue_);
    vkGetDeviceQueue(device.get(), present_qfm, 0, &present_queue_); 
}

void SwapchainPresenter::submit_and_present(SwapchainContext &swapchain_ctx) {
    size_t constexpr swapchains_count = 1;
    auto const &image_ctx = swapchain_ctx.get_image();
    std::array<VkSemaphore, swapchains_count> submit_semaphores{image_ctx.get_submit_semaphore()};
    std::array<VkSemaphore, swapchains_count> present_semaphores{image_ctx.get_present_semaphore()};
    std::array<VkPipelineStageFlags, swapchains_count> stage_flags{VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    std::array<VkCommandBuffer, swapchains_count> command_buffers{image_ctx.get_command_buffer()};
    std::array<VkSwapchainKHR, swapchains_count> swapchains{swapchain_ctx.get_swapchain()};
    std::array<uint32_t, swapchains_count> image_indices;

    VkFence sync_fence = sync_fence_.get();
    vk_assert(vkWaitForFences(device_.get(), 1, &sync_fence, VK_TRUE, timeout), "Failed to wait for fences.");
    vk_assert(vkResetFences(device_.get(), 1, &sync_fence), "Failed to reset the fences.");

    for (size_t i = 0; i < swapchains.size(); ++i) {
        vk_assert(vkAcquireNextImageKHR(device_.get(), swapchains[i], timeout, submit_semaphores[i], nullptr, &image_indices[i]),
                  "Failed to acquire next image.");
    }

    VkSubmitInfo submit_info{
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = static_cast<uint32_t>(submit_semaphores.size()),
        .pWaitSemaphores = submit_semaphores.data(),
        .pWaitDstStageMask = stage_flags.data(),
        .commandBufferCount = static_cast<uint32_t>(command_buffers.size()),
        .pCommandBuffers = command_buffers.data(),
        .signalSemaphoreCount = static_cast<uint32_t>(present_semaphores.size()),
        .pSignalSemaphores = present_semaphores.data(),
    };
    vk_assert(vkQueueSubmit(graphics_queue_, 1, &submit_info, sync_fence), "Failed to submit the queue.");

    VkPresentInfoKHR present_info{
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = static_cast<uint32_t>(present_semaphores.size()),
        .pWaitSemaphores = present_semaphores.data(),
        .swapchainCount = static_cast<uint32_t>(swapchains.size()),
        .pSwapchains = swapchains.data(),
        .pImageIndices = image_indices.data(),
        .pResults = nullptr,
    };
    vk_assert(vkQueuePresentKHR(present_queue_, &present_info), "Failed to present the queue.");
}