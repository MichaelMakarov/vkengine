#include "vertex.hpp"

VkVertexInputBindingDescription Vertex::get_binding_description(uint32_t binding) {
    return VkVertexInputBindingDescription{
        .binding = binding,
        .stride = sizeof(Vertex),
        .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
    };
}

std::array<VkVertexInputAttributeDescription, 4> Vertex::get_attribute_description(uint32_t binding) {
    return {
        VkVertexInputAttributeDescription{
            .location = 0,
            .binding = binding,
            .format = VK_FORMAT_R32G32B32_SFLOAT,
            .offset = offsetof(Vertex, point),
        },
        VkVertexInputAttributeDescription{
            .location = 1,
            .binding = binding,
            .format = VK_FORMAT_R32G32B32_SFLOAT,
            .offset = offsetof(Vertex, normal),
        },
        VkVertexInputAttributeDescription{
            .location = 2,
            .binding = binding,
            .format = VK_FORMAT_R32G32B32_SFLOAT,
            .offset = offsetof(Vertex, color),
        },
        VkVertexInputAttributeDescription{
            .location = 3,
            .binding = binding,
            .format = VK_FORMAT_R32G32_SFLOAT,
            .offset = offsetof(Vertex, texture),
        },
    };
}
