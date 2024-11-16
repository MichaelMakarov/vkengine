#pragma once

#include "memory_block.hpp"

class AllocatorInterface {
  public:
    virtual ~AllocatorInterface() = default;

    virtual MemoryBlock allocate(VkMemoryRequirements const &requirements, VkMemoryAllocateFlags flags) = 0;

    virtual void deallocate(MemoryBlock const &block) = 0;
};
