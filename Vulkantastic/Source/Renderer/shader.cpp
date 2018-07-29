#include "shader.h"
#include "../File/File.h"
#include "core.h"
#include "device.h"
#include "shader_reflection.h"

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
	mUniforms = Reflection.GetUniforms();
	mPushConstants = Reflection.GetPushConstants();

}

Shader::~Shader()
{
	const Device* VulkanDevice = VulkanCore::Get().GetDevice();

	vkDestroyShaderModule(VulkanDevice->GetDevice(), mModule, nullptr);
}
