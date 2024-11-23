#version 450 

layout (location = 0) in vec3 in_color;
layout (location = 1) in vec2 in_texture;

layout (location = 0) out vec4 out_color;

layout (binding = 1) uniform sampler2D image;

void main() {
    vec4 image_color = texture(image, in_texture);
    if (image_color.a == 0) {
        out_color = vec4(in_color, 1);
    } else {
        out_color = image_color;
    }
}