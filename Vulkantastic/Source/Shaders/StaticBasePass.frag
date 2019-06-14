#version 450
#extension GL_ARB_separate_shader_objects : enable

// Same as in image_descriptor_manager.h
#define MaxImages 4096
#define MaxSamplers 2

layout(location=0) out vec4 Color;
layout(location=1) out vec4 Normal;
layout(location=2) out vec4 Position;

layout(location=0) in vec2 fTexCoord;
layout(location=1) in vec3 fNormal;
layout(location=2) in vec4 fPosition;

layout(set = 0, binding = 0) uniform sampler SamplersArray[MaxSamplers];
layout(set = 0, binding = 1) uniform texture2D ImagesArray[MaxImages];

layout(push_constant) uniform PushConstant2
{
	vec3 CustomColor;
    int SmpIdx;
    int ImgIdx;
};

void main()
{
    //vec4 TexColor = texture(sampler2D(ImagesArray[ImgIdx], SamplersArray[SmpIdx]), fTexCoord);

    Color = vec4(CustomColor, 1.0f);// * TexColor;
    Normal = vec4(fNormal,1.0f);
    Position = fPosition;
}