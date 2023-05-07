#include <renderer.hpp>

std::unique_ptr<command_pool_t> make_command_pool(std::shared_ptr<device_t> const &device, uint32_t qfm_index, VkCommandPoolCreateFlags flags)
{
    VkCommandPoolCreateInfo info{.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
                                 .pNext = nullptr,
                                 .flags = flags,
                                 .queueFamilyIndex = qfm_index};
    VkCommandPool pool;
    vk_assert(vkCreateCommandPool(device.get(), &info, nullptr, &pool), "Failed to create command pool.");
    return std::unique_ptr<command_pool_t>(pool, std::default_delete<command_pool_t>{device});
}

void allocate_command_buffers(VkDevice device, VkCommandPool cmdpool, std::vector<VkCommandBuffer> &buffers)
{
    VkCommandBufferAllocateInfo info{.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
                                     .commandPool = cmdpool,
                                     .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                                     .commandBufferCount = static_cast<uint32_t>(buffers.size())};
    vk_assert(vkAllocateCommandBuffers(device, &info, buffers.data()), "Failed to allocate command buffers.");
}

void free_command_buffers(VkDevice device, VkCommandPool cmdpool, std::vector<VkCommandBuffer> const &buffers)
{
    vkFreeCommandBuffers(device, cmdpool, static_cast<uint32_t>(buffers.size()), buffers.data());
}