#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location=0) in vec2 Position;
layout(location=1) in vec2 TexCoord;

layout(location=1) out vec2 fTexCoord;

out gl_PerVertex {
    vec4 gl_Position;
};

void main()
{
	gl_Position  = vec4(Position,0.0f, 1.0f);
	fTexCoord = TexCoord;
}