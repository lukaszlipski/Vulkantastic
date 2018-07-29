#pragma once
#define VK_PROTOTYPES 1
#define VK_USE_PLATFORM_WIN32_KHR 1
#include "vulkan/vulkan.h"
#include "shader_reflection.h"
#include <string>

class Shader
{
public:
	Shader(const std::string& Name);
	~Shader();

	std::string GetName() const { return mName; }
	ShaderType GetType() const { mType; }
	bool IsValid() const { return mModule != VK_NULL_HANDLE; }

private:
	std::string mName;
	ShaderType mType;
	VkShaderModule mModule = VK_NULL_HANDLE;

	std::vector<Input> mInputs;
	std::vector<Uniform> mUniforms;
	std::vector<Uniform> mPushConstants;


};