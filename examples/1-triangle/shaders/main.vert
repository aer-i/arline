#version 460
#extension GL_EXT_buffer_reference : require
#extension GL_EXT_buffer_reference_uvec2 : require

struct Vertex{ float x, y, z, r, g, b; };

layout(constant_id = 0) const uint vboAddrLo = 0;
layout(constant_id = 1) const uint vboAddrHi = 0;

layout(location = 0) out vec3 fragColor;

layout(buffer_reference) restrict readonly buffer VertexBuffer
{
    Vertex vertices[];
};

void main()
{
    VertexBuffer vbo = VertexBuffer(uvec2(vboAddrLo, vboAddrHi));
    Vertex v = vbo.vertices[gl_VertexIndex];

    gl_Position = vec4(v.x, v.y, v.z, 1.0);
    fragColor = vec3(v.r, v.g, v.b);
}