#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location=0) out vec4 Color;
layout(location=1) out vec4 Normal;
layout(location=2) out vec4 Position;

layout(location=0) in vec2 fTexCoord;
layout(location=1) in vec3 fNormal;
layout(location=2) in vec4 fPosition;

//layout(binding=1) uniform sampler2D Albedo;

layout(push_constant) uniform PushConstant2
{
	layout(offset = 128) vec3 CustomColor;
};

void main()
{
    //Color = vec4(CustomColor * texture(Albedo,fTexCoord).rgb, 1.0f);
    Color = vec4(CustomColor, 1.0f);
    Normal = vec4(fNormal,1.0f);
    Position = fPosition;
}