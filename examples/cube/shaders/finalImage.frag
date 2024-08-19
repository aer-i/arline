#version 460

layout(binding = 0) uniform sampler2D textures[];

layout(location = 0) in vec2 inUV;
layout(location = 0) out vec4 outColor;

void main()
{
    outColor = texture(textures[1], inUV);
}