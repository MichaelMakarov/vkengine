#include "image_context.hpp"

#include "graphics_manager.hpp"

ImageContext::ImageContext(shared_ptr_of<VkDevice> device, context_info const &info, shared_ptr_of<VkCommandPool> cmd_pool)
    : image_view_{GraphicsManager::make_image_view(device, info.image, info.format)}
    , framebuffer_{GraphicsManager::make_framebuffer(device, image_view_.get(), info.render_pass, info.format, info.extent)}
    , submit_semaphore_{GraphicsManager::make_semaphore(device)}
    , present_semaphore_{GraphicsManager::make_semaphore(device)}
    , command_buffer_{GraphicsManager::make_command_buffer(device, cmd_pool)} {
}