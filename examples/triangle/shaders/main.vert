#version 460
#extension GL_EXT_buffer_reference : require

layout(location = 0) out vec2 outTextureCoords;

layout(buffer_reference) restrict readonly buffer VertexBuffer
{
    vec2 data[];
};

layout(push_constant) uniform PushConstant
{
    VertexBuffer vb;
};

void main()
{
    vec2 pos = vb.data[gl_VertexIndex * 2 + 0];
    vec2 uv  = vb.data[gl_VertexIndex * 2 + 1];

    gl_Position = vec4(pos, 0.0, 1.0);
    outTextureCoords = uv;
}