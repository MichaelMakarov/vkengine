#version 450 
#extension GL_ARB_separate_shader_objects : enable


layout (location = 0) in vec3 in_color;
layout (location = 1) in vec2 in_texture;

layout (location = 0) out vec4 out_color;

layout (binding = 1) uniform sampler2D image;

void main() {
    //out_color = vec4(in_color, 1);
    //out_color = vec4(in_texture, 0, 1);
    out_color = texture(image, in_texture);
}