#pragma once

#include "graphics/allocator_interface.hpp"
#include "graphics/commander.hpp"
#include "graphics/descriptor_interface.hpp"
#include "graphics/graphics_types.hpp"
#include "graphics/texture_image.hpp"

class TextureDescriptor : public DescriptorInterface {
    TextureImage texture_;
    VkDescriptorImageInfo image_info_;

  public:
    TextureDescriptor(shared_ptr_of<VkDevice> device,
                      std::shared_ptr<AllocatorInterface> allocator,
                      Commander &transfer,
                      Commander &barrier);

    VkDescriptorSetLayoutBinding get_binding() const override;

    VkWriteDescriptorSet get_write(VkDescriptorSet descriptor_set, size_t image_index) const override;
};