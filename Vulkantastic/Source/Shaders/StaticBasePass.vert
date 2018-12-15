#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location=0) in vec3 Position;
layout(location=1) in vec2 TexCoord;
layout(location=2) in vec3 Normal;
layout(location=3) in vec3 Tangent;

layout(location=1) out vec2 fTexCoord;

out gl_PerVertex {
    vec4 gl_Position;
};

layout(push_constant) uniform PushConstant
{
	layout(offset = 0) mat4 MVP;
};

void main()
{
	gl_Position  = MVP * vec4(Position, 1.0f);
	fTexCoord = TexCoord;
}