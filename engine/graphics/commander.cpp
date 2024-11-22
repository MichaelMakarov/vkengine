#include "commander.hpp"

#include "graphics/graphics_error.hpp"
#include "graphics/graphics_manager.hpp"

unique_ptr_of<VkCommandBuffer> Commander::begin_command() {
    auto command_buffer = GraphicsManager::make_command_buffer(device_, command_pool_);
    VkCommandBufferBeginInfo begin_info{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };
    vk_assert(vkBeginCommandBuffer(command_buffer.get(), &begin_info), "Failed to begin command buffer.");
    return command_buffer;
}

void Commander::end_command(VkCommandBuffer command_buffer) {
    vk_assert(vkEndCommandBuffer(command_buffer), "Failed to end command buffer.");

    VkSubmitInfo submit_info{
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &command_buffer,
    };
    vkQueueSubmit(queue_, 1, &submit_info, nullptr);
    vkQueueWaitIdle(queue_);
}

Commander::Commander(shared_ptr_of<VkDevice> device, uint32_t qfm_index, uint32_t queue_index)
    : device_{device}
    , command_pool_{GraphicsManager::make_command_pool(device, VK_COMMAND_POOL_CREATE_TRANSIENT_BIT, qfm_index)} {
    vkGetDeviceQueue(device.get(), qfm_index, queue_index, &queue_);
}

Commander::Commander(shared_ptr_of<VkDevice> device, shared_ptr_of<VkCommandPool> command_pool, VkQueue queue)
    : device_{device}
    , command_pool_{command_pool}
    , queue_{queue} {
}

void Commander::add_command(std::unique_ptr<CommandInterface> command) {
    commands_.push_back(std::move(command));
}

void Commander::execute() {
    if (commands_.empty()) {
        return;
    }
    auto command_buffer = begin_command();
    for (auto &command : commands_) {
        command->execute(command_buffer.get());
        command.release();
    }
    end_command(command_buffer.get());
    commands_.clear();
}
