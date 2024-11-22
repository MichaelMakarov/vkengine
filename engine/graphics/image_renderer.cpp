#include "image_renderer.hpp"

#include "graphics_error.hpp"

VkCommandBuffer ImageRenderer::begin_render_pass() {
    vk_assert(vkResetCommandBuffer(command_buffer_, 0), "Failed to reset the command buffer.");
    {
        VkCommandBufferBeginInfo begin_info{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT,
            .pInheritanceInfo = nullptr,
        };
        vk_assert(vkBeginCommandBuffer(command_buffer_, &begin_info), "Failed to begin the command buffer.");
    }
    {
        VkClearValue clear_value{.color = VkClearColorValue{0, 0, 0, 1}};
        VkRenderPassBeginInfo begin_info{
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            .renderPass = render_pass_,
            .framebuffer = framebuffer_,
            .renderArea = rect_,
            .clearValueCount = static_cast<uint32_t>(clear_values_.size()),
            .pClearValues = clear_values_.data(),
        };
        vkCmdBeginRenderPass(command_buffer_, &begin_info, VkSubpassContents::VK_SUBPASS_CONTENTS_INLINE);
    }
    return command_buffer_;
}

void ImageRenderer::end_render_pass() {
    vkCmdEndRenderPass(command_buffer_);
    vk_assert(vkEndCommandBuffer(command_buffer_), "Failed to end the command buffer.");
}
