#include "pipeline.h"
#include "shader.h"
#include "pipeline_creation.h"
#include "../Utilities/assert.h"

PipelineShaders::PipelineShaders(std::initializer_list<Shader*> Shaders)
{
	for (auto& Shader : Shaders)
	{
		switch (Shader->GetType())
		{
		case ShaderType::VERTEX:
		{
			mVertex = Shader;
			mShaders.push_back(mVertex);
			break;
		}
		case ShaderType::FRAGMENT:
		{
			mFragment = Shader;
			mShaders.push_back(mFragment);
			break;
		}
		default:
		{
			Assert(false); // Unsupported shader type
		}
		}
	}
}

ComputePipeline::ComputePipeline(Shader* ComputeShader)
{
	auto Device = VulkanCore::Get().GetDevice()->GetDevice();

	auto ShadersList = { ComputeShader };
	mDescriptorManager = std::make_unique<DescriptorManager>(ShadersList);

	VkPipelineLayoutCreateInfo ComputeLayoutCreateInfo = {};
	ComputeLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	ComputeLayoutCreateInfo.setLayoutCount = 1;
	auto DescLayout = mDescriptorManager->GetLayout();
	ComputeLayoutCreateInfo.pSetLayouts = &DescLayout;

	Assert(vkCreatePipelineLayout(Device, &ComputeLayoutCreateInfo, nullptr, &mPipelineLayout) == VK_SUCCESS);

	VkComputePipelineCreateInfo ComputePipelineCreateInfo = {};
	ComputePipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
	ComputePipelineCreateInfo.layout = mPipelineLayout;

	VkPipelineShaderStageCreateInfo ComputeShaderStage = {};
	ComputeShaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	ComputeShaderStage.module = ComputeShader->GetModule();
	ComputeShaderStage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
	ComputeShaderStage.pName = "main";
	ComputePipelineCreateInfo.stage = ComputeShaderStage;

	VkPipeline ComputePipeline = nullptr;
	Assert(vkCreateComputePipelines(Device, VK_NULL_HANDLE, 1, &ComputePipelineCreateInfo, nullptr, &mPipeline) == VK_SUCCESS);

}

ComputePipeline::~ComputePipeline()
{
	auto Device = VulkanCore::Get().GetDevice()->GetDevice();

	if (mPipeline)
	{
		vkDestroyPipeline(Device, mPipeline, nullptr);
	}

	if (mPipelineLayout)
	{
		vkDestroyPipelineLayout(Device, mPipelineLayout, nullptr);
	}
}

