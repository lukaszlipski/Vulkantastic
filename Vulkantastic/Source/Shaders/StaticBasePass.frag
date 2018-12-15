#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location=0) out vec4 Color;
layout(location=1) in vec2 fTexCoord;

layout(binding=0) uniform sampler2D Albedo;

layout(push_constant) uniform PushConstant2
{
	layout(offset = 80) vec3 CustomColor;
};

void main()
{
    Color = vec4(CustomColor * texture(Albedo,fTexCoord).rgb, 1.0f);
}