#version 450
#extension GL_ARB_separate_shader_objects : enable


layout(location=0) in vec2 Position;
layout(location=1) in vec3 Color;
layout(location=2) in vec2 Offset;
layout(location=3) in vec2 TexCoord;

layout(location=0) out vec3 fColor;
layout(location=1) out vec2 fTexCoord;

layout(binding=0) uniform UniformBuffer
{
    vec2 Offset;
} UBInstance;

out gl_PerVertex {
    vec4 gl_Position;
};

void main()
{
    gl_Position  = vec4(Position + Offset + UBInstance.Offset, 0.0f, 1.0f);
	fColor = Color;
	fTexCoord = TexCoord;
}