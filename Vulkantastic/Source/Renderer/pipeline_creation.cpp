#include "pipeline_creation.h"
#include "shader.h"
#include "vertex_definitions.h"
#include "core.h"
#include "device.h"
#include <algorithm>
#include "../Utilities/assert.h"
#include "descriptor_manager.h"
#include <array>

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

	BlendDef::BlendDef(AttachmentFlag Components, BlendMode Mode)
	{
		mState.colorWriteMask = static_cast<VkColorComponentFlags>(Components);
		mState.blendEnable = VK_FALSE;

		switch (Mode)
		{
		case BlendMode::Translucent:
		{
			mState.blendEnable = VK_TRUE;

			mState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
			mState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
			mState.colorBlendOp = VK_BLEND_OP_ADD;

			mState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
			mState.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
			mState.alphaBlendOp = VK_BLEND_OP_ADD;
			break;
		}
		case BlendMode::Additive:
		{
			mState.blendEnable = VK_TRUE;

			mState.dstColorBlendFactor = VK_BLEND_FACTOR_DST_ALPHA;
			mState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
			mState.colorBlendOp = VK_BLEND_OP_ADD;

			mState.dstAlphaBlendFactor = VK_BLEND_FACTOR_DST_ALPHA;
			mState.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
			mState.alphaBlendOp = VK_BLEND_OP_ADD;
			break;
		}
		}

	}

	ColorBlendState::ColorBlendState(const std::vector<BlendDef>& AttachmentDefinitions)
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

	ViewportState::ViewportState(const std::vector<ViewportSize>& Sizes)
	{
		mViewports.reserve(Sizes.size());
		mScissors.reserve(Sizes.size());

		for (auto& Size : Sizes)
		{
			VkViewport Viewport = {};
			Viewport.width = Size.Width;
			Viewport.height = Size.Height;
			Viewport.maxDepth = 1.0f;
			Viewport.minDepth = 0.0f;

			VkRect2D Scissor = {};
			Scissor.extent = { static_cast<uint32_t>(Size.Width), static_cast<uint32_t>(Size.Height) };
			Scissor.offset = { 0,0 };
			
			mViewports.push_back(Viewport);
			mScissors.push_back(Scissor);
		}

		mViewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		mViewportState.scissorCount = static_cast<uint32_t>(mScissors.size());
		mViewportState.pScissors = mScissors.data();
		mViewportState.viewportCount = static_cast<uint32_t>(mViewports.size());
		mViewportState.pViewports = mViewports.data();

	}

	PipelineLayout::PipelineLayout(DescriptorManager* DescManager)
	{
		auto Device = VulkanCore::Get().GetDevice()->GetDevice();

		VkPipelineLayoutCreateInfo PipelineLayoutInfo = {};
		PipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		PipelineLayoutInfo.setLayoutCount = 1;
		auto DescLayout = DescManager->GetLayout();
		PipelineLayoutInfo.pSetLayouts = &DescLayout;

		auto PushConstants = DescManager->GetPushConstants();
		std::vector<VkPushConstantRange> PushConstantRanges;
		PushConstantRanges.reserve(PushConstants.size());

		for (auto& ShaderConstants : PushConstants)
		{
			auto& Constants = ShaderConstants.second;

			if(Constants.empty()) { continue; }

			const auto& FirstElem = Constants.front();
			const auto& LastElem = Constants.back();
			const auto& FirstPushConstant = FirstElem.Members.front();
			const auto& LastPushConstant = LastElem.Members.back();

			VkPushConstantRange Range = {};
			Range.stageFlags = ShaderReflection::InternalShaderTypeToVulkan(ShaderConstants.first);
			Range.offset = FirstPushConstant.Offset;
			Range.size = LastPushConstant.Offset + ShaderReflection::GetSizeForFormat(LastPushConstant.Format) - FirstPushConstant.Offset;

			PushConstantRanges.push_back(Range);
		}
	
		PipelineLayoutInfo.pushConstantRangeCount = PushConstantRanges.size();
		PipelineLayoutInfo.pPushConstantRanges = PushConstantRanges.data();

		Assert(vkCreatePipelineLayout(Device, &PipelineLayoutInfo, nullptr, &mPipelineLayout) == VK_SUCCESS);

	}

	PipelineLayout::~PipelineLayout()
	{
		auto Device = VulkanCore::Get().GetDevice()->GetDevice();

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