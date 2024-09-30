#include "vertex.hpp"

VkVertexInputBindingDescription vertex2d::get_binding_description(uint32_t binding) {
    return VkVertexInputBindingDescription{
        .binding = binding,
        .stride = sizeof(vertex2d),
        .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
    };
}

std::array<VkVertexInputAttributeDescription, 2> vertex2d::get_attribute_description(uint32_t binding) {
    std::array<VkVertexInputAttributeDescription, 2> descriptions;
    descriptions[0] = VkVertexInputAttributeDescription{
        .location = 0,
        .binding = binding,
        .format = VK_FORMAT_R32G32_SFLOAT,
        .offset = offsetof(vertex2d, point_),
    };
    descriptions[1] = VkVertexInputAttributeDescription{
        .location = 1,
        .binding = binding,
        .format = VK_FORMAT_R32G32B32_SFLOAT,
        .offset = offsetof(vertex2d, color_),
    };
    return descriptions;
}

VkVertexInputBindingDescription vertex3d::get_binding_description(uint32_t binding) {
    return VkVertexInputBindingDescription{
        .binding = binding,
        .stride = sizeof(vertex3d),
        .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
    };
}

std::array<VkVertexInputAttributeDescription, 3> vertex3d::get_attribute_description(uint32_t binding) {
    std::array<VkVertexInputAttributeDescription, 3> descriptions;
    descriptions[0] = VkVertexInputAttributeDescription{
        .location = 0,
        .binding = binding,
        .format = VK_FORMAT_R32G32B32_SFLOAT,
        .offset = offsetof(vertex3d, point_),
    };
    descriptions[1] = VkVertexInputAttributeDescription{
        .location = 1,
        .binding = binding,
        .format = VK_FORMAT_R32G32B32_SFLOAT,
        .offset = offsetof(vertex3d, normal_),
    };
    descriptions[2] = VkVertexInputAttributeDescription{
        .location = 2,
        .binding = binding,
        .format = VK_FORMAT_R32G32B32_SFLOAT,
        .offset = offsetof(vertex3d, color_),
    };
    return descriptions;
}
