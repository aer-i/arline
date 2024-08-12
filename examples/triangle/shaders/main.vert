#version 460
//#extension GL_EXT_buffer_reference : require

const vec2 positions[] = {
    vec2( 0.5, -0.5),
    vec2(-0.5, -0.5),
    vec2( 0.0,  0.5)
};

void main()
{
    gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
}