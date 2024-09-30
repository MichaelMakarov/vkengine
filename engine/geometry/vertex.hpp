#pragma once

#include <glm/glm.hpp>
#include <vulkan/vulkan.hpp>

#include <array>

class vertex2d {
    glm::vec2 point_;
    glm::vec3 color_;

  public:
    vertex2d(glm::vec2 const &point, glm::vec3 const &color)
        : point_{point}
        , color_{color} {
    }

    vertex2d()
        : vertex2d(glm::vec2(0.f, 0.f), glm::vec3(1.f, 1.f, 1.f)) {
    }

    glm::vec2 const &get_point() const {
        return point_;
    }

    void set_point(glm::vec2 const &point) {
        point_ = point;
    }

    glm::vec3 const &get_color() const {
        return color_;
    }

    void set_color(glm::vec3 const &color) {
        color_ = color;
    }

    static VkVertexInputBindingDescription get_binding_description(uint32_t binding);

    static std::array<VkVertexInputAttributeDescription, 2> get_attribute_description(uint32_t binding);
};

class vertex3d {
    glm::vec3 point_;
    glm::vec3 normal_;
    glm::vec3 color_;

  public:
    vertex3d(glm::vec3 const &point, glm::vec3 normal, glm::vec3 color)
        : point_{point}
        , normal_{normal}
        , color_{color} {
    }

    vertex3d()
        : vertex3d(glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 0.f, 0.f), glm::vec3(1.f, 1.f, 1.f)) {
    }

    glm::vec3 const &get_point() const {
        return point_;
    }

    void set_point(glm::vec3 const &point) {
        point_ = point;
    }

    glm::vec3 const &get_normal() const {
        return normal_;
    }

    void set_normal(glm::vec3 const &normal) {
        normal_ = normal;
    }

    glm::vec3 const &get_color() const {
        return color_;
    }

    void set_color(glm::vec3 const &color) {
        color_ = color;
    }

    static VkVertexInputBindingDescription get_binding_description(uint32_t binding);

    static std::array<VkVertexInputAttributeDescription, 3> get_attribute_description(uint32_t binding);
};