#pragma once

#include "geometry/vertex.hpp"
#include "graphics/memory_buffer.hpp"

#include <vector>

class PlainMesh {
    std::vector<Vertex2d> vertices_;
    std::vector<uint16_t> indices_;
    std::vector<MemoryBuffer> buffers_;

  public:
    PlainMesh(shared_ptr_of<VkDevice> device, VkPhysicalDevice phys_device, uint32_t transfer_qfm);

    void draw(VkCommandBuffer command_buffer);
};