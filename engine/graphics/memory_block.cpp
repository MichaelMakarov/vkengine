#include "memory_block.hpp"

VkDeviceSize MemoryBlock::get_offset(VkDeviceSize alignment) const {
    return (offset_ + alignment - 1) & (~(alignment - 1));
}

VkDeviceSize MemoryBlock::get_size(VkDeviceSize alignment) const {
    return size_ + offset_ - get_offset(alignment);
}

bool operator<(MemoryBlock const &left, MemoryBlock const &right) {
    if (left.memory_ < right.memory_) {
        return true;
    }
    if (left.memory_ == right.memory_) {
        return left.offset_ < right.offset_;
    }
    return false;
}