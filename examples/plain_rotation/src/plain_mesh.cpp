#include "plain_mesh.hpp"

#include "graphics/buffer_copy_command.hpp"

namespace {

    template <typename T>
    size_t get_data_size(std::vector<T> const &arr) {
        return arr.size() * sizeof(T);
    }

} // namespace

PlainMesh::PlainMesh(shared_ptr_of<VkDevice> device, std::shared_ptr<AllocatorInterface> allocator, Commander &transfer) {
    mesh_ = Mesh<Vertex, uint16_t>(
        {
            /* first plane */
            Vertex{
                .point = glm::vec3(-0.5f, 0.5f, 0.0f),
                .color = glm::vec3(1.0f, 0.0f, 0.0f),
                .texture = glm::vec2(1.0f, 0.0f),
            },
            Vertex{
                .point = glm::vec3(0.5f, 0.5f, 0.0f),
                .color = glm::vec3(0.0f, 1.0f, 0.0f),
                .texture = glm::vec2(0.0f, 0.0f),
            },
            Vertex{
                .point = glm::vec3(0.5f, -0.5f, 0.0f),
                .color = glm::vec3(0.0f, 0.0f, 1.0f),
                .texture = glm::vec2(0.0f, 1.0f),
            },
            Vertex{
                .point = glm::vec3(-0.5f, -0.5f, 0.0f),
                .color = glm::vec3(1.0f, 1.0f, 1.0f),
                .texture = glm::vec2(1.0f, 1.0f),
            },
            /* second plane */
            Vertex{
                .point = glm::vec3(-0.5f, 0.5f, -0.5f),
                .color = glm::vec3(1.0f, 0.0f, 0.0f),
                .texture = glm::vec2(1.0f, 0.0f),
            },
            Vertex{
                .point = glm::vec3(0.5f, 0.5f, -0.5f),
                .color = glm::vec3(0.0f, 1.0f, 0.0f),
                .texture = glm::vec2(0.0f, 0.0f),
            },
            Vertex{
                .point = glm::vec3(0.5f, -0.5f, -0.5f),
                .color = glm::vec3(0.0f, 0.0f, 1.0f),
                .texture = glm::vec2(0.0f, 1.0f),
            },
            Vertex{
                .point = glm::vec3(-0.5f, -0.5f, -0.5f),
                .color = glm::vec3(1.0f, 1.0f, 1.0f),
                .texture = glm::vec2(1.0f, 1.0f),
            },
        },
        {/* first plane */ 0, 2, 1, 2, 0, 3, /* second plane */ 4, 6, 5, 6, 4, 7});
    vertex_buffer_ = MemoryBuffer(device,
                                  allocator,
                                  VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                  mesh_.get_vertex_data_size(),
                                  VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    index_buffer_ = MemoryBuffer(device,
                                 allocator,
                                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                 mesh_.get_index_data_size(),
                                 VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
    MemoryBuffer vertex_buffer(device,
                               allocator,
                               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                               mesh_.get_vertex_data_size(),
                               VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    MemoryBuffer index_buffer(device,
                              allocator,
                              VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                              mesh_.get_index_data_size(),
                              VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    vertex_buffer.fill(mesh_.get_vertex_data(), mesh_.get_vertex_data_size());
    index_buffer.fill(mesh_.get_index_data(), mesh_.get_index_data_size());
    transfer.add_command(
        std::make_unique<BufferCopyCommand>(vertex_buffer.get_buffer(), vertex_buffer_.get_buffer(), vertex_buffer.get_size()));
    transfer.add_command(
        std::make_unique<BufferCopyCommand>(index_buffer.get_buffer(), index_buffer_.get_buffer(), index_buffer.get_size()));
    transfer.execute();
}

void PlainMesh::draw(VkCommandBuffer command_buffer) const {
    VkBuffer vertex_buffer = vertex_buffer_.get_buffer();
    VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(command_buffer, 0, 1, &vertex_buffer, &offset);
    vkCmdBindIndexBuffer(command_buffer, index_buffer_.get_buffer(), 0, VK_INDEX_TYPE_UINT16);
    vkCmdDrawIndexed(command_buffer, static_cast<uint32_t>(mesh_.get_index_count()), 1, 0, 0, 0);
}

VkVertexInputBindingDescription PlainMesh::get_vertex_binding_description() const {
    return VkVertexInputBindingDescription{
        .binding = 0,
        .stride = sizeof(Vertex),
        .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
    };
}

std::vector<VkVertexInputAttributeDescription> PlainMesh::get_vertex_attribute_descriptions() const {
    return {
        VkVertexInputAttributeDescription{
            .location = 0,
            .binding = 0,
            .format = VK_FORMAT_R32G32B32_SFLOAT,
            .offset = offsetof(Vertex, point),
        },
        VkVertexInputAttributeDescription{
            .location = 1,
            .binding = 0,
            .format = VK_FORMAT_R32G32B32_SFLOAT,
            .offset = offsetof(Vertex, color),
        },
        VkVertexInputAttributeDescription{
            .location = 2,
            .binding = 0,
            .format = VK_FORMAT_R32G32_SFLOAT,
            .offset = offsetof(Vertex, texture),
        },
    };
}
