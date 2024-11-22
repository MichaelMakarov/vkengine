#include "matrix_descriptor.hpp"

#include "graphics/graphics_manager.hpp"

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace {

    struct Matrices {
        glm::mat4x4 model;
        glm::mat4x4 view;
        glm::mat4x4 proj;
    };

} // namespace

MatrixDescriptor::MatrixDescriptor(shared_ptr_of<VkDevice> device)
    : device_{device}
    , tn_{std::chrono::steady_clock::now()} {
}

void MatrixDescriptor::setup_buffers(size_t buffers_count, VkExtent2D extent, std::shared_ptr<AllocatorInterface> allocator) {
    if (buffers_count > buffers_.size()) {
        buffers_.reserve(buffers_count);
        buffer_infos_.reserve(buffers_count);
        while (buffers_.size() < buffers_count) {
            auto &buffer = buffers_.emplace_back(device_,
                                                 allocator,
                                                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                                 sizeof(Matrices),
                                                 VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
            buffer_infos_.emplace_back(VkDescriptorBufferInfo{.buffer = buffer.get_buffer(), .offset = 0, .range = sizeof(Matrices)});
        }
    }
    extent_ = extent;
}

void MatrixDescriptor::update_content(size_t index) {
    auto seconds = std::chrono::duration<float, std::chrono::seconds::period>(std::chrono::steady_clock::now() - tn_);
    Matrices matrices{
        .model = glm::rotate(glm::mat4x4(1.0f), seconds.count() * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
        .view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
        .proj = glm::perspective(glm::radians(45.0f), extent_.width / static_cast<float>(extent_.height), 0.1f, 10.0f),
    };
    matrices.proj[1][1] = -matrices.proj[1][1];
    buffers_[index].fill(&matrices, sizeof(Matrices));
}

VkDescriptorSetLayoutBinding MatrixDescriptor::get_binding() const {
    return VkDescriptorSetLayoutBinding{
        .binding = 0,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
        .pImmutableSamplers = nullptr,
    };
}

VkWriteDescriptorSet MatrixDescriptor::get_write(VkDescriptorSet descriptor_set, size_t image_index) const {
    return VkWriteDescriptorSet{
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = descriptor_set,
        .dstBinding = 0,
        .dstArrayElement = 0,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .pImageInfo = nullptr,
        .pBufferInfo = &buffer_infos_[image_index],
        .pTexelBufferView = nullptr,
    };
}