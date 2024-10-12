#include "plain_mesh.hpp"

#include "graphics/buffer_manager.hpp"

namespace {

    template <typename T>
    size_t get_data_size(std::vector<T> const &arr) {
        return arr.size() * sizeof(T);
    }

} // namespace

PlainMesh::PlainMesh(shared_ptr_of<VkDevice> device, VkPhysicalDevice phys_device, uint32_t transfer_qfm) {
    vertices_ = {
        Vertex2d{.point = glm::vec2(-0.5f, 0.5f), .color = glm::vec3(1.0f, 0.0f, 0.0f)},
        Vertex2d{.point = glm::vec2(0.5f, 0.5f), .color = glm::vec3(0.0f, 1.0f, 0.0f)},
        Vertex2d{.point = glm::vec2(0.5f, -0.5f), .color = glm::vec3(0.0f, 0.0f, 1.0f)},
        Vertex2d{.point = glm::vec2(-0.5f, -0.5f), .color = glm::vec3(1.0f, 1.0f, 1.0f)},
    };
    indices_ = {0, 2, 1, 2, 0, 3};
    buffers_ = MemoryBuffer::make_buffers(device,
                                          phys_device,
                                          {
                                              MemoryBuffer::Config{
                                                  .size = get_data_size(vertices_),
                                                  .usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                                              },
                                              MemoryBuffer::Config{
                                                  .size = get_data_size(indices_),
                                                  .usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                                              },
                                          },
                                          VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    auto staging_buffers = MemoryBuffer::make_buffers(device,
                                                      phys_device,
                                                      {
                                                          MemoryBuffer::Config{
                                                              .size = get_data_size(vertices_),
                                                              .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                                          },
                                                          MemoryBuffer::Config{
                                                              .size = get_data_size(indices_),
                                                              .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                                          },
                                                      },
                                                      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    staging_buffers[0].fill(vertices_.data(), get_data_size(vertices_));
    staging_buffers[1].fill(indices_.data(), get_data_size(indices_));
    BufferManager(device, transfer_qfm).copy_buffers(staging_buffers.data(), buffers_.data(), buffers_.size());
}

void PlainMesh::draw(VkCommandBuffer command_buffer) {
    VkBuffer vertex_buffer = buffers_[0].get_buffer();
    VkDeviceSize offset = 0;
    VkBuffer index_buffer = buffers_[1].get_buffer();
    vkCmdBindVertexBuffers(command_buffer, 0, 1, &vertex_buffer, &offset);
    vkCmdBindIndexBuffer(command_buffer, index_buffer, 0, VK_INDEX_TYPE_UINT16);
    vkCmdDrawIndexed(command_buffer, static_cast<uint32_t>(indices_.size()), 1, 0, 0, 0);
}
