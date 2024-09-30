#version 450

layout (location = 0) out vec3 color;

const vec2 points[3] = vec2[] (
    vec2(1, 1),
    vec2(0, -1),
    vec2(-1, 1)
);

const vec3 colors[3] = vec3[] (
    vec3(1, 0, 0),
    vec3(0, 1, 0),
    vec3(0, 0, 1)
);

void main() {
    gl_Position = vec4(points[gl_VertexIndex], 0, 1);
    color = colors[gl_VertexIndex];
}