#version 460

layout(binding = 0) uniform sampler2D textures[1];

layout(location = 0) in  vec2 inTexCoords;
layout(location = 0) out vec4 outColor;

void main()
{
    outColor = texture(textures[0], inTexCoords);
}