#include "texture_descriptor.hpp"

#include "image/image_wrapper.hpp"

#include "graphics/image_copy_command.hpp"
#include "graphics/memory_barrier.hpp"
#include "graphics/memory_barrier_command.hpp"
#include "graphics/memory_buffer.hpp"

TextureDescriptor::TextureDescriptor(shared_ptr_of<VkDevice> device,
                                     std::shared_ptr<AllocatorInterface> allocator,
                                     Commander &transfer,
                                     Commander &barrier) {
    ImageWrapper image("avocado.png");
    texture_ = TextureImage(device, allocator, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, image.get_width(), image.get_height());
    MemoryBuffer buffer(device,
                        allocator,
                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                        image.get_size(),
                        VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    buffer.fill(image.get_data(), image.get_size());
    auto barrier_command = std::make_unique<MemoryBarrierCommand>(VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);
    barrier_command->add_image_barrier(MemoryBarrier::make_image_barrier(texture_.get_image(),
                                                                         MemoryBarrier::ImageInfo{
                                                                             .access_mask = 0,
                                                                             .layout = VK_IMAGE_LAYOUT_UNDEFINED,
                                                                         },
                                                                         MemoryBarrier::ImageInfo{
                                                                             .access_mask = VK_ACCESS_TRANSFER_WRITE_BIT,
                                                                             .layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                                                         }));
    transfer.add_command(std::move(barrier_command));
    transfer.add_command(
        std::make_unique<ImageCopyCommand>(buffer.get_buffer(), texture_.get_image(), image.get_width(), image.get_height()));
    transfer.execute();
    barrier_command = std::make_unique<MemoryBarrierCommand>(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
    barrier_command->add_image_barrier(MemoryBarrier::make_image_barrier(texture_.get_image(),
                                                                         MemoryBarrier::ImageInfo{
                                                                             .access_mask = VK_ACCESS_TRANSFER_WRITE_BIT,
                                                                             .layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                                                         },
                                                                         MemoryBarrier::ImageInfo{
                                                                             .access_mask = VK_ACCESS_SHADER_READ_BIT,
                                                                             .layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                                                         }));
    barrier.add_command(std::move(barrier_command));
    barrier.execute();
    image_info_ = VkDescriptorImageInfo{
        .sampler = texture_.get_sampler(),
        .imageView = texture_.get_image_view(),
        .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
    };
}

VkDescriptorSetLayoutBinding TextureDescriptor::get_binding() const {
    return VkDescriptorSetLayoutBinding{
        .binding = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
        .pImmutableSamplers = nullptr,
    };
}

VkWriteDescriptorSet TextureDescriptor::get_write(VkDescriptorSet descriptor_set, size_t image_index) const {
    return VkWriteDescriptorSet{
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = descriptor_set,
        .dstBinding = 1,
        .dstArrayElement = 0,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .pImageInfo = &image_info_,
        .pBufferInfo = nullptr,
        .pTexelBufferView = nullptr,
    };
}
