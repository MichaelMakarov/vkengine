#pragma once

#include "geometry/mesh.hpp"
#include "graphics/allocator_interface.hpp"
#include "graphics/commander.hpp"
#include "graphics/memory_buffer.hpp"

#include <glm/glm.hpp>

#include <vector>

class PlainMesh {
    struct Vertex {
        glm::vec3 point;
        glm::vec3 color;
        glm::vec2 texture;
    };
    Mesh<Vertex, uint16_t> mesh_;
    MemoryBuffer vertex_buffer_;
    MemoryBuffer index_buffer_;

  public:
    PlainMesh(shared_ptr_of<VkDevice> device, std::shared_ptr<AllocatorInterface> allocator, Commander &transfer);

    void draw(VkCommandBuffer command_buffer) const;

    VkVertexInputBindingDescription get_vertex_binding_description() const;

    std::vector<VkVertexInputAttributeDescription> get_vertex_attribute_descriptions() const;
};