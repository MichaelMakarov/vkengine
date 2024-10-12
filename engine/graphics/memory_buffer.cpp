#include "memory_buffer.hpp"

#include "graphics_error.hpp"
#include "graphics_manager.hpp"

#include <unordered_map>

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

    constexpr VkDeviceSize align_memory_size(VkDeviceSize size, VkDeviceSize align) {
        return (size + align - 1) & (~(align - 1));
    }

    struct MemoryInfo {
        VkDeviceSize size;
        shared_ptr_of<VkDeviceMemory> memory;

        friend bool operator<(MemoryInfo const &left, MemoryInfo const &right) {
            return left.size < right.size;
        }
    };

} // namespace

MemoryBuffer::MemoryBuffer(shared_ptr_of<VkDevice> device, Config info)
    : device_{device}
    , buffer_{GraphicsManager::make_buffer(device, info.size, info.usage, info.mode)} {
    vkGetBufferMemoryRequirements(device.get(), buffer_.get(), &requirements_);
}

void MemoryBuffer::bind(shared_ptr_of<VkDeviceMemory> memory) {
    memory_.swap(memory);
    vk_assert(vkBindBufferMemory(device_.get(), buffer_.get(), memory_.get(), offset_), "Failed to bind buffer to memory.");
}

MemoryBuffer::MemoryBuffer(shared_ptr_of<VkDevice> device, VkPhysicalDevice phys_device, Config info, VkMemoryPropertyFlags properties)
    : MemoryBuffer(device, info) {
    offset_ = 0;
    uint32_t type_index = get_memory_type_index(phys_device, requirements_.memoryTypeBits, properties);
    bind(GraphicsManager::make_device_memory(device, requirements_.size, type_index));
}

std::vector<MemoryBuffer> MemoryBuffer::make_buffers(shared_ptr_of<VkDevice> device,
                                                     VkPhysicalDevice phys_device,
                                                     std::vector<Config> const &configs,
                                                     VkMemoryPropertyFlags properties) {
    std::vector<MemoryBuffer> buffers;
    buffers.reserve(configs.size());
    std::unordered_map<uint32_t, MemoryInfo> memory_map;
    for (Config const &config : configs) {
        auto &buffer = buffers.emplace_back(MemoryBuffer(device, config));
        MemoryInfo &info = memory_map[buffer.requirements_.memoryTypeBits];
        buffer.offset_ = align_memory_size(info.size, buffer.requirements_.alignment);
        info.size = buffer.offset_ + buffer.requirements_.size;
    }
    for (auto &[type, info] : memory_map) {
        uint32_t type_index = get_memory_type_index(phys_device, type, properties);
        info.memory = GraphicsManager::make_device_memory(device, info.size, type_index);
    }
    for (auto &buffer : buffers) {
        buffer.bind(memory_map[buffer.requirements_.memoryTypeBits].memory);
    }
    return buffers;
}

void MemoryBuffer::fill(void const *data, size_t size) {
    void *ptr;
    vk_assert(vkMapMemory(device_.get(), memory_.get(), offset_, requirements_.size, 0, &ptr),
              "Failed to map memory: offset={}, size={}.",
              offset_,
              requirements_.size);
    std::memcpy(ptr, data, size);
    vkUnmapMemory(device_.get(), memory_.get());
}