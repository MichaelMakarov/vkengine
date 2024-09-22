#pragma once

#include <glm/glm.hpp>

struct vertex2d {
    glm::vec2 point;
    glm::vec3 color;
};

struct vertex3d {
    glm::vec3 point;
    glm::vec3 normal;
    glm::vec3 color;
};