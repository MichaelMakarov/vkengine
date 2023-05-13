#version 450

const vec2 points[3] = vec2[]
(
    vec2(1, 1),
    vec2(0, -1),
    vec2(-1, 1)
);

void main()
{
    gl_Position = vec4(points[gl_VertexIndex], 0, 1);
}