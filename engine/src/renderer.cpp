#include <renderer.hpp>
#include <GLFW/glfw3.h>

constexpr uint64_t timeout = std::numeric_limits<uint64_t>::max();

graphics_renderer::graphics_renderer(std::shared_ptr<GLFWwindow> const &wnd) : _wnd{wnd}
{
    create_core();
}

graphics_renderer::~graphics_renderer()
{
    clear_framebuffers();
}

void graphics_renderer::run()
{
    int w, h;
    glfwGetWindowSize(_wnd.get(), &w, &h);
    on_window_resized(w, h);
    while (!glfwWindowShouldClose(_wnd.get()))
    {
        glfwPollEvents();
        draw_frame();
    }
}

void graphics_renderer::draw_frame()
{
    auto sync_fence = _frame_sync_fences[_frame_index].get();
    auto submit_semaphore = _frame_submit_semaphores[_frame_index].get();
    auto present_semaphore = _frame_present_semaphores[_frame_index].get();
    auto swapchain = _swapchain.get();
    auto frame_cmdbuffer = _frame_cmdbuffers[_frame_index];
    uint32_t image_index;

    vkWaitForFences(_device.get(), 1, &sync_fence, VK_TRUE, timeout);
    vkResetFences(_device.get(), 1, &sync_fence);
    vk_assert(vkAcquireNextImageKHR(_device.get(), _swapchain.get(), timeout, submit_semaphore, nullptr, &image_index), "Failed to acquirean image.");

    VkPipelineStageFlags flags{VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    VkSubmitInfo submit_info{
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &submit_semaphore,
        .pWaitDstStageMask = &flags,
        .commandBufferCount = 1,
        .pCommandBuffers = &_frame_cmdbuffers[image_index],
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &present_semaphore,
    };
    vk_assert(vkQueueSubmit(_graphics_queue, 1, &submit_info, sync_fence), "Failed to submit a queue.");
    VkPresentInfoKHR present_info{
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &present_semaphore,
        .swapchainCount = 1,
        .pSwapchains = &swapchain,
        .pImageIndices = &image_index,
        .pResults = nullptr,
    };
    vk_assert(vkQueuePresentKHR(_present_queue, &present_info), "Failed to present a queue.");

    _frame_index = (_frame_index + 1) % _image_views.size();
}

void graphics_renderer::update_resources()
{
}

void graphics_renderer::set_draw_commands()
{
    stop_drawing();
    for (std::size_t i{}; i < _frame_cmdbuffers.size(); ++i)
    {
        auto frame_cmdbuffer = _frame_cmdbuffers[i];
        auto framebuffer = _framebuffers[i].get();
        vk_assert(vkResetCommandBuffer(frame_cmdbuffer, 0), "Failed to reset a frame command buffer.");
        {
            VkCommandBufferBeginInfo begin_info{
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                .flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT,
                .pInheritanceInfo = nullptr,
            };
            vk_assert(vkBeginCommandBuffer(frame_cmdbuffer, &begin_info), "Failed to begin command buffer.");
        }
        {
            VkRect2D rect{.offset = {0, 0}, .extent = _extent};
            VkClearValue clear_value{.color = VkClearColorValue{0, 0, 0, 1}};
            VkRenderPassBeginInfo begin_info{
                .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
                .renderPass = _render_pass.get(),
                .framebuffer = framebuffer,
                .renderArea = rect,
                .clearValueCount = 1,
                .pClearValues = &clear_value,
            };
            vkCmdBeginRenderPass(frame_cmdbuffer, &begin_info, VkSubpassContents::VK_SUBPASS_CONTENTS_INLINE);
        }
        for (auto &drawer : _pipeline_drawers)
        {
            drawer->draw_command(frame_cmdbuffer);
        }
        vkCmdEndRenderPass(frame_cmdbuffer);
        vk_assert(vkEndCommandBuffer(frame_cmdbuffer), "Failed to end a frame command buffer.");
    }
}

void graphics_renderer::on_window_resized(int w, int h)
{
    _extent.width = static_cast<uint32_t>(w);
    _extent.height = static_cast<uint32_t>(h);
    create_swapchain();
    create_pipeline();
    set_draw_commands();
}

void graphics_renderer::stop_drawing()
{
    vk_assert(vkDeviceWaitIdle(_device.get()), "Failed to wait idles to free frame command buffers.");
}