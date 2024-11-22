#include "graphics_types.hpp"

class MemoryBlock {
    shared_ptr_of<VkDeviceMemory> memory_;
    VkDeviceSize offset_;
    VkDeviceSize size_;

  public:
    MemoryBlock(shared_ptr_of<VkDeviceMemory> memory, VkDeviceSize offset, VkDeviceSize size)
        : memory_{memory}
        , offset_{offset}
        , size_{size} {
    }

    MemoryBlock()
        : MemoryBlock(nullptr, 0, 0) {
    }

    shared_ptr_of<VkDeviceMemory> const &get_memory() const {
        return memory_;
    }

    VkDeviceSize get_offset(VkDeviceSize alignment = 1) const;

    VkDeviceSize get_size(VkDeviceSize alignment = 1) const;

    explicit operator bool() const {
        return static_cast<bool>(memory_);
    }

    friend bool operator<(MemoryBlock const &left, MemoryBlock const &right);
};