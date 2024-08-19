#version 460
#extension GL_EXT_buffer_reference : require

layout(buffer_reference) restrict readonly buffer PB
{
    float positions[];
};

layout(buffer_reference) restrict readonly buffer CB
{
    float colors[];
};

layout(buffer_reference) restrict readonly buffer SB
{
    mat4 cameraProjectionMatrix;
    mat4 cubeMatrix;
};

layout(push_constant) uniform PushConstant
{
    PB pb;
    CB cb;
    SB sb;
};

layout(location = 0) out vec3 outColor;

void main()
{
    gl_Position = sb.cameraProjectionMatrix * sb.cubeMatrix * vec4(
        pb.positions[gl_VertexIndex * 3 + 0],
        pb.positions[gl_VertexIndex * 3 + 1],
        pb.positions[gl_VertexIndex * 3 + 2],
        1.0
    );

    outColor = vec3(
        cb.colors[gl_VertexIndex * 3 + 0],
        cb.colors[gl_VertexIndex * 3 + 1],
        cb.colors[gl_VertexIndex * 3 + 2]
    );
}