#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location=0) out vec4 Color;
layout(location=0) in vec3 fColor;
layout(location=1) in vec2 fTexCoord;

layout(binding=1) uniform sampler2D Image;

layout(binding=2) buffer UniformBuffer2
{
    float Test[];
} UBInstance2;

layout(push_constant) uniform PushConstant2
{
	layout(offset = 16) vec3 CustomColor;
};

void main()
{
    Color = vec4(fColor * texture(Image,fTexCoord).rgb * CustomColor * UBInstance2.Test[0], 1.0f);
}