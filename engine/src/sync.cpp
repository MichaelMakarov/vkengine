#include <renderer.hpp>

std::unique_ptr<fence_t> make_fence(std::shared_ptr<device_t> const &device)
{
    VkFenceCreateInfo info{.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
                           .flags = VK_FENCE_CREATE_SIGNALED_BIT};
    VkFence fence;
    vk_assert(vkCreateFence(device.get(), &info, nullptr, &fence), "Failed to create a fence.");
    return std::unique_ptr<fence_t>(fence, std::default_delete<fence_t>{device});
}

std::unique_ptr<semaphore_t> make_semaphore(std::shared_ptr<device_t> const &device)
{
    VkSemaphoreCreateInfo info{.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
    VkSemaphore semaphore;
    vk_assert(vkCreateSemaphore(device.get(), &info, nullptr, &semaphore), "Failed to create a semaphore.");
    return std::unique_ptr<semaphore_t>(semaphore, std::default_delete<semaphore_t>{device});
}