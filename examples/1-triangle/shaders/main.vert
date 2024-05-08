#version 460
#extension GL_EXT_buffer_reference : require
#extension GL_EXT_buffer_reference_uvec2 : require

struct Vertex{ float x, y, z, r, g, b; };

layout(location = 0) out vec3 fragColor;

layout(buffer_reference) restrict readonly buffer VertexBuffer
{
    Vertex vertices[];
};

layout(push_constant) uniform PushConstant
{
    VertexBuffer vbo;
};


void main()
{
    Vertex v = vbo.vertices[gl_VertexIndex];

    gl_Position = vec4(v.x, v.y, v.z, 1.0);
    fragColor = vec3(v.r, v.g, v.b);
}