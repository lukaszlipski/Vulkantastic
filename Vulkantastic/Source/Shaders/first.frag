#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location=0) out vec4 Color;
layout(location=0) in vec3 fCol;
layout(location=1) in vec2 fTexCoords;

layout(binding=1) uniform sampler2D Image;

void main()
{
    Color = vec4(texture(Image,fTexCoords).rgb,1.0f);
}