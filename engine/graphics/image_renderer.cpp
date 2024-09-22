#include "image_renderer.hpp"

#include "graphics_error.hpp"

image_renderer::image_renderer(image_context const *image_ptr, VkRenderPass render_pass, VkExtent2D extent)
    : image_ptr_{image_ptr}
    , render_pass_{render_pass}
    , rect_{.offset = {0, 0}, .extent = extent} {
    if (image_ptr == nullptr) {
        raise_error("Image context pointer is null");
    }
}

VkCommandBuffer image_renderer::begin_render_pass() {
    VkCommandBuffer command_buffer = image_ptr_->get_command_buffer();
    vk_assert(vkResetCommandBuffer(command_buffer, 0), "Failed to reset the command buffer.");
    {
        VkCommandBufferBeginInfo begin_info{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT,
            .pInheritanceInfo = nullptr,
        };
        vk_assert(vkBeginCommandBuffer(command_buffer, &begin_info), "Failed to begin the command buffer.");
    }
    {
        VkClearValue clear_value{.color = VkClearColorValue{0, 0, 0, 1}};
        VkRenderPassBeginInfo begin_info{
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            .renderPass = render_pass_,
            .framebuffer = image_ptr_->get_framebuffer(),
            .renderArea = rect_,
            .clearValueCount = 1,
            .pClearValues = &clear_value,
        };
        vkCmdBeginRenderPass(command_buffer, &begin_info, VkSubpassContents::VK_SUBPASS_CONTENTS_INLINE);
    }
    return command_buffer;
}

void image_renderer::end_render_pass() {
    VkCommandBuffer command_buffer = image_ptr_->get_command_buffer();
    vkCmdEndRenderPass(command_buffer);
    vk_assert(vkEndCommandBuffer(command_buffer), "Failed to end the command buffer.");
}
