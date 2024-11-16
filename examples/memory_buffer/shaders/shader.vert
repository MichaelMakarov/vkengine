#version 450

layout(location = 0) in vec2 in_point;
layout(location = 1) in vec3 in_color;
layout(location = 2) in vec2 in_texture;

layout(location = 0) out vec3 out_color;
layout(location = 1) out vec2 out_texture;

layout(binding = 0) uniform Matrices {
    mat4 model;
    mat4 view;
    mat4 proj;
} matrices;

void main() {
    gl_Position = matrices.proj * matrices.view * matrices.model * vec4(in_point, 0, 1);
    out_color = in_color;
    out_texture = in_texture;
}