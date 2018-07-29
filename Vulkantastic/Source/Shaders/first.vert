#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding=0) uniform UniformBuffer
{
    vec2 Fake;
	vec2 Offset;
    float Array[5];
} UBInstance;

layout(binding=2, push_constant) uniform PushConstant
{
    bool Test[100];
} PCInstance;

layout(location=0) in vec2 aPos;
layout(location=1) in vec3 aCol;
layout(location=2) in vec2 aTexCoords;

layout(location=0) out vec3 fCol;
layout(location=1) out vec2 fTexCoords;

out gl_PerVertex {
    vec4 gl_Position;
};



void main()
{
    gl_Position  = vec4(aPos + UBInstance.Offset, 0.0f, 1.0f);
	fCol = aCol;
    fTexCoords = aTexCoords;
}