#include "memory_buffer.hpp"

#include "graphics_error.hpp"
#include "graphics_manager.hpp"

namespace {

    uint32_t get_memory_type_index(VkPhysicalDevice device, uint32_t type, VkMemoryPropertyFlags properties) {
        VkPhysicalDeviceMemoryProperties memory_properties;
        vkGetPhysicalDeviceMemoryProperties(device, &memory_properties);
        for (uint32_t i = 0; i < memory_properties.memoryTypeCount; ++i) {
            if ((type & (1 << i)) && (memory_properties.memoryTypes[i].propertyFlags & properties) == properties) {
                return i;
            }
        }
        raise_error("Failed to find suitable memory: type={}, properties={}.", type, properties);
    }

} // namespace

memory_buffer::memory_buffer(shared_ptr_of<VkDevice> device, VkPhysicalDevice phys_device, config const &info)
    : device_{device}
    , buffer_{graphics_manager::make_buffer(device, info.size, info.usage, info.mode)} {
    vkGetBufferMemoryRequirements(device.get(), buffer_.get(), &requirements_);
    uint32_t type_index = get_memory_type_index(phys_device, requirements_.memoryTypeBits, info.properties);
    memory_ = graphics_manager::make_device_memory(device, requirements_.size, type_index);
    vk_assert(vkBindBufferMemory(device.get(), buffer_.get(), memory_.get(), 0), "Failed to bind buffer to memory.");
}

void memory_buffer::fill(void const *data, size_t size) {
    void *ptr;
    vk_assert(vkMapMemory(device_.get(), memory_.get(), 0, requirements_.size, 0, &ptr), "Failed to map memory.");
    std::memcpy(ptr, data, size);
    vkUnmapMemory(device_.get(), memory_.get());
}