#include "pipeline.h"
#include "shader.h"
#include "vertex_definitions.h"
#include "core.h"
#include "device.h"
#include <algorithm>
#include "../Utilities/assert.h"

namespace PipelineCreation
{
	ShaderPipeline::ShaderPipeline(Shader* Vertex, Shader* Fragment)
	{
		mShaderPipeline.resize(2);

		mShaderPipeline[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		mShaderPipeline[0].module = Vertex->GetModule();
		mShaderPipeline[0].pName = "main";
		mShaderPipeline[0].stage = VK_SHADER_STAGE_VERTEX_BIT;

		mShaderPipeline[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		mShaderPipeline[1].module = Fragment->GetModule();
		mShaderPipeline[1].pName = "main";
		mShaderPipeline[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;

	}

	DepthStencilState::DepthStencilState(DepthCompareOP CompareOP /*= DepthCompareOP::LESS*/, bool EnableWrite /*= true*/, bool EnableTest /*= true*/, bool EnableStencil /*= false*/, float Min /*= 0.0f*/, float Max /*= 1.0f*/)
	{
		mDepthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		mDepthStencil.depthBoundsTestEnable = VK_FALSE;

		switch (CompareOP)
		{
		case DepthCompareOP::LESS:
		{
			mDepthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
			break;
		}
		default:
		{
			Assert(false); // Not supported operation
		}
		}

		mDepthStencil.depthTestEnable = EnableTest;
		mDepthStencil.depthWriteEnable = EnableWrite;
		mDepthStencil.maxDepthBounds = Max;
		mDepthStencil.minDepthBounds = Min;
		mDepthStencil.stencilTestEnable = EnableStencil;
	}

	BlendDef::BlendDef(AttachmentFlag Components, bool EnableBlend /*= false*/)
	{
		mState.colorWriteMask = static_cast<VkColorComponentFlags>(Components);
		mState.blendEnable = EnableBlend;
	}

	ColorBlendState::ColorBlendState(std::initializer_list<BlendDef> AttachmentDefinitions)
	{
		mColorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		mColorBlendState.logicOpEnable = VK_FALSE;

		for (auto& Def : AttachmentDefinitions)
		{
			mAttachemntStates.push_back(Def.GetState());
		}

		mColorBlendState.attachmentCount = static_cast<uint32_t>(mAttachemntStates.size());
		mColorBlendState.pAttachments = mAttachemntStates.data();

	}

	RasterizationState::RasterizationState(CullMode Cull /*= CullMode::BACK*/, FrontFace FaceDir /*= FrontFace::CLOCKWISE*/)
	{
		mRasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		mRasterizationState.cullMode = static_cast<VkCullModeFlags>(Cull);
		mRasterizationState.frontFace = static_cast<VkFrontFace>(FaceDir);
		mRasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
		mRasterizationState.lineWidth = 1.0f;
		mRasterizationState.depthClampEnable = VK_FALSE;
		mRasterizationState.depthBiasEnable = VK_FALSE;
		mRasterizationState.rasterizerDiscardEnable = VK_FALSE;
	}

	MultisampleState::MultisampleState(bool Enable /*= false*/, uint8_t SampleCount /*= 1*/)
	{
		mMultisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		mMultisampleState.sampleShadingEnable = Enable;
		mMultisampleState.rasterizationSamples = static_cast<VkSampleCountFlagBits>(SampleCount);
	}

	InputAssemblyState::InputAssemblyState()
	{
		mInputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		mInputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		mInputAssemblyState.primitiveRestartEnable = VK_FALSE;
	}

	VertexInputState::VertexInputState(Shader* VertexShader, std::initializer_list<VertexFormatDeclaration> VertexFormats)
	{
		std::vector<Input> ShaderInputs = VertexShader->GetInputs();

		mVertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

		auto VertexFormat = VertexFormats.begin();
		for (uint32_t i = 0; i < VertexFormats.size(); ++i, ++VertexFormat)
		{
			VkVertexInputBindingDescription VertBindDesc = {};
			VertBindDesc.binding = i;
			VertBindDesc.inputRate = VertexFormat->Instance ? VK_VERTEX_INPUT_RATE_INSTANCE : VK_VERTEX_INPUT_RATE_VERTEX;
			VertBindDesc.stride = VertexFormat->Size;

			mInputBindings.push_back(VertBindDesc);

			for (auto& Member : VertexFormat->Members)
			{
				auto ShaderInput = std::find_if(ShaderInputs.begin(), ShaderInputs.end(), [&Name = Member.Name](Input& Entry){
					return Name == Entry.Name;
				});

				Assert(ShaderInput != ShaderInputs.end()); // Cannot find shader input

				VkVertexInputAttributeDescription Attribute = {};
				Attribute.format = ShaderReflection::InternalFormatToVulkan(ShaderInput->Format);
				Attribute.location = ShaderInput->Location;
				Attribute.binding = i;
				Attribute.offset = Member.Offset;

				mInputsAttributs.push_back(Attribute);
			}

		}

		mVertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		mVertexInputState.vertexBindingDescriptionCount = static_cast<uint32_t>(mInputBindings.size());
		mVertexInputState.pVertexBindingDescriptions = mInputBindings.data();
		mVertexInputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(mInputsAttributs.size());
		mVertexInputState.pVertexAttributeDescriptions = mInputsAttributs.data();

	}

	PipelineLayout::PipelineLayout(std::initializer_list<Shader*> Shaders)
	{
		auto Device = VulkanCore::Get().GetDevice()->GetDevice();

		std::vector<VkDescriptorSetLayoutBinding> DescLayoutBindings;

		for (auto& Shader : Shaders)
		{
			auto Uniforms = Shader->GetUniforms();

			for (auto& Uniform : Uniforms)
			{
				VkDescriptorSetLayoutBinding Binding = {};
				Binding.binding = Uniform.Binding;
				Binding.descriptorCount = 1;
				Binding.stageFlags = ShaderReflection::InternalShaderTypeToVulkan(Shader->GetType());
				Binding.descriptorType = ShaderReflection::InternalUniformTypeToVulkan(Uniform.Format);

				DescLayoutBindings.push_back(Binding);
			}

		}

		VkDescriptorSetLayoutCreateInfo DescriptorLayoutInfo = {};
		DescriptorLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		DescriptorLayoutInfo.bindingCount = static_cast<uint32_t>(DescLayoutBindings.size());
		DescriptorLayoutInfo.pBindings = DescLayoutBindings.data();

		Assert(vkCreateDescriptorSetLayout(Device, &DescriptorLayoutInfo, nullptr, &mDescriptorLayout) == VK_SUCCESS);

		VkPipelineLayoutCreateInfo PipelineLayoutInfo = {};
		PipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		PipelineLayoutInfo.setLayoutCount = 1;
		PipelineLayoutInfo.pSetLayouts = &mDescriptorLayout;

		Assert(vkCreatePipelineLayout(Device, &PipelineLayoutInfo, nullptr, &mPipelineLayout) == VK_SUCCESS);

	}

	PipelineLayout::~PipelineLayout()
	{
		auto Device = VulkanCore::Get().GetDevice()->GetDevice();

		vkDestroyDescriptorSetLayout(Device, mDescriptorLayout, nullptr);
		vkDestroyPipelineLayout(Device, mPipelineLayout, nullptr);
	}

	DynamicState::DynamicState()
	{
		mDynamicStates.push_back(VK_DYNAMIC_STATE_VIEWPORT);

		mDynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		mDynamicState.dynamicStateCount = static_cast<uint32_t>(mDynamicStates.size());
		mDynamicState.pDynamicStates = mDynamicStates.data();
	}


}