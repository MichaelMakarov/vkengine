#pragma once

#include "allocator_interface.hpp"

#include <set>
#include <unordered_map>

class MemoryListAllocator : public AllocatorInterface {
    shared_ptr_of<VkDevice> device_;
    VkPhysicalDeviceMemoryProperties memory_properties_;
    std::unordered_map<uint32_t, std::set<MemoryBlock>> type_to_memory_;
    std::unordered_map<shared_ptr_of<VkDeviceMemory>, uint32_t> memory_to_type_;

    MemoryBlock extend(uint32_t type_index, VkDeviceSize required_size);

    uint32_t memory_type_index(VkMemoryAllocateFlags memory_flags, uint32_t memory_type) const;

  public:
    MemoryListAllocator(shared_ptr_of<VkDevice> device, VkPhysicalDevice phys_device);

    MemoryBlock allocate(VkMemoryRequirements const &requirements, VkMemoryAllocateFlags flags) override;

    void deallocate(MemoryBlock const &block) override;
};