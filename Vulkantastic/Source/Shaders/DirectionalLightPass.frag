#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location=0) out vec4 Color;

layout(location=1) in vec2 fTexCoord;

layout(binding=0) uniform sampler2D ColorTex;
layout(binding=1) uniform sampler2D NormalTex;
layout(binding=2) uniform sampler2D PositionTex;

layout(push_constant) uniform LightInfo
{
	layout(offset = 0) vec3 Direction;
    vec3 LightColor;
};

void main()
{
    vec3 LocalColor = texture(ColorTex,fTexCoord).rgb;
    vec3 LocalNormal = texture(NormalTex,fTexCoord).rgb;

    vec3 Diffuse = LocalColor * LightColor * max(dot(-Direction, LocalNormal), 0.0f);
    Color = vec4(Diffuse, 1.0f);
}