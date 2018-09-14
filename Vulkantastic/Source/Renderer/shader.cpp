#include "shader.h"
#include "../File/File.h"
#include "core.h"
#include "device.h"
#include "shader_reflection.h"
#include "../File/path.h"

Shader::Shader(const std::string& Name)
	: mName(Name)
{

	FileGuard SourceHandle(File::Get().OpenRead(mName));

	int64_t SourceSize = File::Get().Size(SourceHandle.Get());
	if (!SourceSize) { return; }

	std::vector<uint32_t> Source(SourceSize / sizeof(uint32_t));
	SourceHandle->Read(reinterpret_cast<uint8_t*>(Source.data()), static_cast<int32_t>(SourceSize));

	VkShaderModuleCreateInfo CreateInfo = {};
	CreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	CreateInfo.codeSize = SourceSize;
	CreateInfo.pCode = Source.data();

	const Device* VulkanDevice = VulkanCore::Get().GetDevice();
	if (vkCreateShaderModule(VulkanDevice->GetDevice(), &CreateInfo, nullptr, &mModule) != VK_SUCCESS)
	{
		return;
	}

	ShaderReflection Reflection(std::move(Source));

	mType = Reflection.GetShaderType();
	mInputs = Reflection.GetInputs();
	mOutputs = Reflection.GetOutputs();
	mUniforms = Reflection.GetUniforms();
	mPushConstants = Reflection.GetPushConstants();

}

Shader::~Shader()
{
	const Device* VulkanDevice = VulkanCore::Get().GetDevice();

	vkDestroyShaderModule(VulkanDevice->GetDevice(), mModule, nullptr);
}

bool ShaderManager::Startup()
{
	const std::string ShaderDirectory = File::Get().CurrentDirectory() + "/Shaders/";
	std::vector<std::string> ShaderFileList;

	File::Get().GetFiles(ShaderFileList, ShaderDirectory);

	for (auto& ShaderFile : ShaderFileList)
	{
		std::string ShaderPath = ShaderDirectory + ShaderFile;
		Shader* NewShader = new Shader(ShaderPath);
		if (!NewShader->IsValid())
		{
			// #TODO: Log
			return false;
		}
		mShaderList[ShaderFile] = NewShader;
	}

	return true;
}

bool ShaderManager::Shutdown()
{
	for (auto& ShaderToDelete : mShaderList)
	{
		delete ShaderToDelete.second;
	}

	return true;
}

Shader* ShaderManager::Find(std::string Name)
{
	if (Path::GetExtension(Name) != "spv")
	{
		Name += ".spv";
	}
	return mShaderList[Name];
}
