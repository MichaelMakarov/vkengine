#include "buffer_manager.hpp"

#include "graphics_error.hpp"
#include "graphics_manager.hpp"

#include <array>

BufferManager::BufferManager(shared_ptr_of<VkDevice> device, uint32_t transfer_qfm)
    : device_{device}
    , command_pool_{GraphicsManager::make_command_pool(device, VK_COMMAND_POOL_CREATE_TRANSIENT_BIT, transfer_qfm)} {
    vkGetDeviceQueue(device.get(), transfer_qfm, 0, &queue_);
}

void BufferManager::copy_buffers(MemoryBuffer const *src_buffers, MemoryBuffer const *dst_buffers, size_t buffers_count) {
    auto command_buffer = GraphicsManager::make_command_buffer(device_, command_pool_);

    VkCommandBufferBeginInfo begin_info{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };
    vk_assert(vkBeginCommandBuffer(command_buffer.get(), &begin_info), "Failed to begin command buffer for memory copying.");

    for (size_t i = 0; i < buffers_count; ++i) {
        VkBufferCopy buffer_copy{
            .srcOffset = 0,
            .dstOffset = 0,
            .size = src_buffers[i].get_size(),
        };
        vkCmdCopyBuffer(command_buffer.get(), src_buffers[i].get_buffer(), dst_buffers[i].get_buffer(), 1, &buffer_copy);
    }

    vk_assert(vkEndCommandBuffer(command_buffer.get()), "Failed to end command buffer for memory copying.");

    std::array<VkCommandBuffer, 1> command_buffers{command_buffer.get()};
    VkSubmitInfo submit_info{
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = static_cast<uint32_t>(command_buffers.size()),
        .pCommandBuffers = command_buffers.data(),
    };
    vkQueueSubmit(queue_, 1, &submit_info, nullptr);
    vkQueueWaitIdle(queue_);
}
