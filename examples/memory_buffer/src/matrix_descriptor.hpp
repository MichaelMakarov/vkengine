#pragma once

#include "graphics/allocator_interface.hpp"
#include "graphics/descriptor_interface.hpp"
#include "graphics/memory_buffer.hpp"

#include <chrono>

class MatrixDescriptor : public DescriptorInterface {
    shared_ptr_of<VkDevice> device_;
    std::vector<MemoryBuffer> buffers_;
    std::vector<VkDescriptorBufferInfo> buffer_infos_;
    VkExtent2D extent_;
    std::chrono::steady_clock::time_point const tn_;

  public:
    explicit MatrixDescriptor(shared_ptr_of<VkDevice> device);

    void setup_buffers(size_t buffers_count, VkExtent2D extent, std::shared_ptr<AllocatorInterface> allocator);

    void update_content(size_t index);

    VkDescriptorSetLayoutBinding get_binding() const override;

    VkWriteDescriptorSet get_write(VkDescriptorSet descriptor_set, size_t image_index) const override;
};