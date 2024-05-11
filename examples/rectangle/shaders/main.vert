#version 460
#extension GL_EXT_buffer_reference : require

layout(location = 0) out vec3 fragColor;

layout(buffer_reference) restrict readonly buffer VertexBuffer
{
    float data[];
};

layout(buffer_reference) restrict readonly buffer IndexBuffer
{
    uint data[];
};

layout(push_constant) uniform PushConstant
{
    VertexBuffer vbo;
    IndexBuffer ibo;
};

void main()
{
    uint index = ibo.data[gl_VertexIndex];

    vec3 pos = vec3(
        vbo.data[index * 3 + 0],
        vbo.data[index * 3 + 1],
        vbo.data[index * 3 + 2]
    );

    gl_Position = vec4(pos, 1.0);
    fragColor = vec3(1, 0.5, 0.75);
}