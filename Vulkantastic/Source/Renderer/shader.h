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
	ShaderType GetType() const { return mType; }
	bool IsValid() const { return mModule != VK_NULL_HANDLE; }
	VkShaderModule GetModule() const { return mModule; }

	inline std::vector<Input> GetInputs() const { return mInputs; }
	inline std::vector<Output> GetOutputs() const { return mOutputs; }
	inline std::vector<Uniform> GetUniforms() const { return mUniforms; }
	inline std::vector<Uniform> GetPushConstants() const { return mPushConstants; }

private:
	std::string mName;
	ShaderType mType;
	VkShaderModule mModule = VK_NULL_HANDLE;

	std::vector<Input> mInputs;
	std::vector<Output> mOutputs;
	std::vector<Uniform> mUniforms;
	std::vector<Uniform> mPushConstants;

};

class ShaderManager
{
public:
	static ShaderManager& Get()
	{
		static ShaderManager* instance = new ShaderManager();
		return *instance;
	}

	bool Startup();
	bool Shutdown();

	Shader* Find(std::string Name);

private:
	ShaderManager() = default;

	std::map<std::string, Shader*> mShaderList;

};