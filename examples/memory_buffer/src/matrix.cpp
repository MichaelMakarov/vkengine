#include "matrix.hpp"

#include "graphics/graphics_manager.hpp"

#include <glm/gtc/matrix_transform.hpp>

namespace {

    struct Matrices {
        glm::mat4x4 model;
        glm::mat4x4 view;
        glm::mat4x4 proj;
    };

    VkDescriptorSetLayoutBinding get_layout_binding(uint32_t binding) {
        return VkDescriptorSetLayoutBinding{
            .binding = binding,
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
            .pImmutableSamplers = nullptr,
        };
    }

} // namespace

Matrix::Matrix(shared_ptr_of<VkDevice> device, VkPhysicalDevice phys_device)
    : device_{device}
    , phys_device_{phys_device}
    , set_layout_{GraphicsManager::make_descriptor_set_layout(device, {get_layout_binding(0)})}
    , tn_{std::chrono::steady_clock::now()} {
}

void Matrix::setup_buffers(size_t buffers_count, VkExtent2D extent) {
    if (buffers_count > buffers_.size()) {
        std::vector<MemoryBuffer::Config> configs(buffers_count - buffers_.size(),
                                                  MemoryBuffer::Config{
                                                      .size = sizeof(Matrices),
                                                      .usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                                  });
        auto buffers = MemoryBuffer::make_buffers(device_,
                                                  phys_device_,
                                                  configs,
                                                  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        std::move(buffers.begin(), buffers.end(), std::back_inserter(buffers_));
        VkDescriptorPoolSize pool_size{.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, .descriptorCount = static_cast<uint32_t>(buffers_count)};
        descriptor_pool_ = GraphicsManager::make_descriptor_pool(device_, {pool_size});
        descriptor_sets_ = GraphicsManager::allocate_descriptor_sets(device_,
                                                                     descriptor_pool_.get(),
                                                                     std::vector<VkDescriptorSetLayout>(buffers_count, set_layout_.get()));

        for (size_t index = 0; index < descriptor_sets_.size(); ++index) {
            VkDescriptorBufferInfo buffer_info{
                .buffer = buffers_[index].get_buffer(),
                .offset = 0,
                .range = sizeof(Matrices),
            };
            VkWriteDescriptorSet write_descriptor{
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet = descriptor_sets_[index],
                .dstBinding = 0,
                .dstArrayElement = 0,
                .descriptorCount = 1,
                .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .pImageInfo = nullptr,
                .pBufferInfo = &buffer_info,
                .pTexelBufferView = nullptr,
            };
            vkUpdateDescriptorSets(device_.get(), 1, &write_descriptor, 0, nullptr);
        }
    }
}

void Matrix::update_content(size_t index) {
    auto seconds = std::chrono::duration<float, std::chrono::seconds::period>(std::chrono::steady_clock::now() - tn_);
    Matrices matrices{
        .model = glm::rotate(glm::mat4x4(1.0f), seconds.count() * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
        .view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
        .proj = glm::perspective(glm::radians(45.0f), extent_.width / static_cast<float>(extent_.height), 0.1f, 10.0f),
    };
    matrices.proj[1][1] = -matrices.proj[1][1];
    buffers_[index].fill(&matrices, sizeof(Matrices));
}

void Matrix::setup_command_buffer(VkPipelineLayout layout, VkCommandBuffer command_buffer, size_t index) {
    vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 0, 1, &descriptor_sets_[index], 0, nullptr);
}
