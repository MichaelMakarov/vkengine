#version 450

layout (location = 0) in vec2 in_point;
layout (location = 1) in vec3 in_color;

layout (location = 0) out vec3 out_color;

void main() {
    gl_Position = vec4(in_point, 0, 1);
    out_color = in_color;
}