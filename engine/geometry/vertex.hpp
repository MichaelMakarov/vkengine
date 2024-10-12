#pragma once

#include <glm/glm.hpp>
#include <vulkan/vulkan.hpp>

#include <array>

struct Vertex2d {
    glm::vec2 point;
    glm::vec3 color;

    static VkVertexInputBindingDescription get_binding_description(uint32_t binding);

    static std::array<VkVertexInputAttributeDescription, 2> get_attribute_description(uint32_t binding);
};

struct Vertex3d {
    glm::vec3 point;
    glm::vec3 normal;
    glm::vec3 color;
    struct {
        uint8_t u, v;
    } texture;

    static VkVertexInputBindingDescription get_binding_description(uint32_t binding);

    static std::array<VkVertexInputAttributeDescription, 4> get_attribute_description(uint32_t binding);
};
