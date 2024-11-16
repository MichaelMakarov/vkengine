#include "memory_list_allocator.hpp"

#include "graphics_error.hpp"
#include "graphics_manager.hpp"

#include <algorithm>

namespace {

    constexpr inline VkDeviceSize default_size = 1024 * 1024 * 256;
    constexpr inline VkDeviceSize size_divider = 10;

} // namespace

MemoryBlock MemoryListAllocator::extend(uint32_t type_index, VkDeviceSize required_size) {
    uint32_t heap_index = memory_properties_.memoryTypes[type_index].heapIndex;
    auto const &memory_heap = memory_properties_.memoryHeaps[heap_index];
    VkDeviceSize pool_size = std::max(std::min(memory_heap.size / size_divider, default_size), required_size);
    auto memory = GraphicsManager::make_device_memory(device_, pool_size, type_index);
    memory_to_type_[memory] = type_index;
    return MemoryBlock(memory, 0, pool_size);
}

uint32_t MemoryListAllocator::memory_type_index(VkMemoryAllocateFlags memory_flags, uint32_t memory_type) const {
    for (uint32_t i = 0; i < memory_properties_.memoryTypeCount; ++i) {
        if ((memory_type & (1 << i)) && (memory_properties_.memoryTypes[i].propertyFlags & memory_flags) == memory_flags) {
            return i;
        }
    }
    raise_error("Failed to find suitable memory: type={}, flags={}.", memory_type, memory_flags);
}

MemoryBlock extract(MemoryBlock &block, VkMemoryRequirements const &requirements) {
    VkDeviceSize size = requirements.size + block.get_offset(requirements.alignment) - block.get_offset();
    MemoryBlock new_block(block.get_memory(), block.get_offset(), size);
    block = MemoryBlock(block.get_memory(), block.get_offset() + size, block.get_size() - size);
    return new_block;
}

MemoryListAllocator::MemoryListAllocator(shared_ptr_of<VkDevice> device, VkPhysicalDevice phys_device)
    : device_{device} {
    vkGetPhysicalDeviceMemoryProperties(phys_device, &memory_properties_);
    type_to_memory_.reserve(memory_properties_.memoryTypeCount);
    for (uint32_t type_index = 0; type_index < memory_properties_.memoryTypeCount; ++type_index) {
        auto const &memory_type = memory_properties_.memoryTypes[type_index];
        info_println("Memory[{}]: flags={}, heap={}", type_index, memory_type.propertyFlags, memory_type.heapIndex);
    }
    for (uint32_t heap_index = 0; heap_index < memory_properties_.memoryHeapCount; ++heap_index) {
        auto const &memory_heap = memory_properties_.memoryHeaps[heap_index];
        info_println("Heap[{}]: flags={}, size={}", heap_index, memory_heap.flags, memory_heap.size);
    }
}

MemoryBlock MemoryListAllocator::allocate(VkMemoryRequirements const &requirements, VkMemoryAllocateFlags flags) {
    uint32_t type_index = memory_type_index(flags, requirements.memoryTypeBits);
    auto &memory_set = type_to_memory_[type_index];
    auto iter = std::ranges::find_if(memory_set, [&requirements](MemoryBlock const &block) {
        return block.get_size(requirements.alignment) >= requirements.size;
    });
    MemoryBlock block;
    if (iter != memory_set.end()) {
        auto node = memory_set.extract(iter);
        block = extract(node.value(), requirements);
        if (node.value().get_size() > 0) {
            memory_set.insert(std::move(node));
        }
    } else {
        auto page = extend(type_index, requirements.size);
        block = extract(page, requirements);
        if (page.get_size() > 0) {
            memory_set.insert(page);
        }
    }
    info_println("Allocate memory[{}]={}: size={}, alignment={}, offset={}.",
                 type_index,
                 reinterpret_cast<uintptr_t>(block.get_memory().get()),
                 requirements.size,
                 requirements.alignment,
                 block.get_offset());
    return block;
}

void MemoryListAllocator::deallocate(MemoryBlock const &memblock) {
    MemoryBlock block{memblock};
    uint32_t type_index = memory_to_type_[block.get_memory()];
    info_println("Deallocate memory[{}]={}: size={}, offset={}.",
                 type_index,
                 reinterpret_cast<uintptr_t>(memblock.get_memory().get()),
                 block.get_size(),
                 block.get_offset());
    auto &memory_set = type_to_memory_[type_index];
    auto prev = memory_set.lower_bound(block);
    auto next = prev;
    if (prev != memory_set.begin() && (--prev)->get_memory() == block.get_memory()) {
        if (prev->get_offset() + prev->get_size() == block.get_offset()) {
            auto node = memory_set.extract(prev);
            block = MemoryBlock(block.get_memory(), node.value().get_offset(), node.value().get_size() + block.get_size());
        }
    }
    if (next != memory_set.end() && next->get_memory() == block.get_memory()) {
        if (next->get_offset() == block.get_offset() + block.get_size()) {
            auto node = memory_set.extract(next);
            block = MemoryBlock(block.get_memory(), block.get_offset(), node.value().get_size() + block.get_size());
        }
    }
    bool inserted;
    std::tie(std::ignore, inserted) = memory_set.insert(block);
    if (!inserted) {
        raise_error("Failed to deallocate memory block: memory={}, offset={}, size={}",
                    reinterpret_cast<uintptr_t>(memblock.get_memory().get()),
                    memblock.get_offset(),
                    memblock.get_size());
    }
}
