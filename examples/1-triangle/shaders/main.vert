#version 460
#extension GL_EXT_buffer_reference : require

layout(location = 0) out vec3 fragColor;

layout(buffer_reference) restrict readonly buffer VertexBuffer
{
    float data[];
};

layout(push_constant) uniform PushConstant
{
    VertexBuffer vbo;
    float r, g, b;
};

void main()
{
    vec3 pos = vec3(
        vbo.data[gl_VertexIndex * 3 + 0],
        vbo.data[gl_VertexIndex * 3 + 1],
        vbo.data[gl_VertexIndex * 3 + 2]
    );

    gl_Position = vec4(pos, 1.0);
    fragColor = vec3(r, g, b);
}