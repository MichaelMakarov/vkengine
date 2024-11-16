#pragma once

#include "geometry/vertex.hpp"
#include "graphics/allocator_interface.hpp"
#include "graphics/memory_buffer.hpp"
#include "graphics/commander.hpp"

#include <vector>

class PlainMesh {
    std::vector<Vertex2d> vertices_;
    std::vector<uint16_t> indices_;
    MemoryBuffer vertex_buffer_;
    MemoryBuffer index_buffer_;

  public:
    PlainMesh(shared_ptr_of<VkDevice> device, std::shared_ptr<AllocatorInterface> allocator, Commander &transfer);

    void draw(VkCommandBuffer command_buffer) const;

    VkVertexInputBindingDescription get_vertex_binding_description() const;

    std::vector<VkVertexInputAttributeDescription> get_vertex_attribute_descriptions() const;
};