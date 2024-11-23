#pragma once

#include <glm/glm.hpp>
#include <vulkan/vulkan.hpp>

#include <array>

struct Vertex {
    glm::vec3 point;
    glm::vec3 normal;
    glm::vec3 color;
    glm::vec2 texture;

    static VkVertexInputBindingDescription get_binding_description(uint32_t binding);

    static std::array<VkVertexInputAttributeDescription, 4> get_attribute_description(uint32_t binding);
};
