#include "vertex.hpp"

VkVertexInputBindingDescription Vertex2d::get_binding_description(uint32_t binding) {
    return VkVertexInputBindingDescription{
        .binding = binding,
        .stride = sizeof(Vertex2d),
        .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
    };
}

std::array<VkVertexInputAttributeDescription, 2> Vertex2d::get_attribute_description(uint32_t binding) {
    return {
        VkVertexInputAttributeDescription{
            .location = 0,
            .binding = binding,
            .format = VK_FORMAT_R32G32_SFLOAT,
            .offset = offsetof(Vertex2d, point),
        },
        VkVertexInputAttributeDescription{
            .location = 1,
            .binding = binding,
            .format = VK_FORMAT_R32G32B32_SFLOAT,
            .offset = offsetof(Vertex2d, color),
        },
    };
}

VkVertexInputBindingDescription Vertex3d::get_binding_description(uint32_t binding) {
    return VkVertexInputBindingDescription{
        .binding = binding,
        .stride = sizeof(Vertex3d),
        .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
    };
}

std::array<VkVertexInputAttributeDescription, 4> Vertex3d::get_attribute_description(uint32_t binding) {
    return {
        VkVertexInputAttributeDescription{
            .location = 0,
            .binding = binding,
            .format = VK_FORMAT_R32G32B32_SFLOAT,
            .offset = offsetof(Vertex3d, point),
        },
        VkVertexInputAttributeDescription{
            .location = 1,
            .binding = binding,
            .format = VK_FORMAT_R32G32B32_SFLOAT,
            .offset = offsetof(Vertex3d, normal),
        },
        VkVertexInputAttributeDescription{
            .location = 2,
            .binding = binding,
            .format = VK_FORMAT_R32G32B32_SFLOAT,
            .offset = offsetof(Vertex3d, color),
        },
        VkVertexInputAttributeDescription{
            .location = 3,
            .binding = binding,
            .format = VK_FORMAT_R8G8_UNORM,
            .offset = offsetof(Vertex3d, texture),
        },
    };
}
