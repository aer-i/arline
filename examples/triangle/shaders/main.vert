#version 460
#extension GL_EXT_buffer_reference : require

layout(buffer_reference) restrict readonly buffer VB
{
    float positions[];
};

layout(push_constant) uniform PushConstant
{
    VB vb;
};

void main()
{
    gl_Position = vec4(
        vb.positions[gl_VertexIndex * 3 + 0],
        vb.positions[gl_VertexIndex * 3 + 1],
        vb.positions[gl_VertexIndex * 3 + 2],
        1.0
    );
}