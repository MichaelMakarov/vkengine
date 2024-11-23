#pragma once

#include "mesh.hpp"
#include "vertex.hpp"

#include <string_view>
#include <cstdint>

class MeshReader {
    public:
      static Mesh<Vertex, uint32_t> read_blender(std::string_view filename);
};