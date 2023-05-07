#include <renderer.hpp>
#include <vector>

std::unique_ptr<command_pool_t> make_command_pool(std::shared_ptr<device_t> const &device, uint32_t qfm_index, VkCommandPoolCreateFlags flags);
void allocate_command_buffers(VkDevice device, VkCommandPool cmdpool, std::vector<VkCommandBuffer> &buffers);
void free_command_buffers(VkDevice device, VkCommandPool cmdpool, std::vector<VkCommandBuffer> const &buffers);

void renderer::copy_resources()
{
    if (!_copy_descriptors.empty())
        return;
    if (!_copy_cmdpool)
    {
        _copy_cmdpool = make_command_pool(_device, _props.transfer_qfm, 0);
    }
    std::vector<VkCommandBuffer> buffers(_copy_descriptors.size());
    allocate_command_buffers(_device.get(), _copy_cmdpool.get(), buffers);
    std::exception_ptr exptr;
    try
    {
        {
            VkCommandBufferBeginInfo info{.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                                          .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT};
            for (std::size_t i{}; i < buffers.size(); ++i)
            {
                auto buffer = buffers[i];
                auto &desc = _copy_descriptors[i];
                VkBufferCopy copy{.srcOffset = static_cast<VkDeviceSize>(desc.srcoffset),
                                  .dstOffset = static_cast<VkDeviceSize>(desc.dstoffset),
                                  .size = static_cast<VkDeviceSize>(desc.size)};
                vk_assert(vkBeginCommandBuffer(buffer, &info), "Failed to begin command buffer.");
                vkCmdCopyBuffer(buffer, desc.srcbuf, desc.dstbuf, 1, &copy);
                vk_assert(vkEndCommandBuffer(buffer), "Failed to end command buffer.");
            }
        }
        {
            VkSubmitInfo info{.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                              .commandBufferCount = static_cast<uint32_t>(buffers.size()),
                              .pCommandBuffers = buffers.data()};
            auto fence = _copy_sync_fence.get();
            vk_assert(vkResetFences(_device.get(), 1, &fence), "Failed to reset fence.");
            vk_assert(vkQueueSubmit(_transfer_queue, 1, &info, fence), "Failed to submit a queue.");
            vk_assert(vkWaitForFences(_device.get(), 1, &fence, VkBool32{1}, std::numeric_limits<uint64_t>::max()), "Failed to wait for fences.");
        }
    }
    catch (std::exception const ex)
    {
        exptr = std::current_exception();
    }
    free_command_buffers(_device.get(), _copy_cmdpool.get(), buffers);
    if (exptr)
    {
        std::rethrow_exception(exptr);
    }
}