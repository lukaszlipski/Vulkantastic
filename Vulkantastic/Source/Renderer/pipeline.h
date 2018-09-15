#pragma once
#include "core.h"
#include <vector>
#include "vertex_definitions_inc.h"

class Shader;

enum class PipelineStage
{
	START = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
	VERTEX_INPUT = VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
	VERTEX = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT,
	FRAGMENT = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
	COLOR_ATTACHMENT = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
	COMPUTE = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
	TRANSER = VK_PIPELINE_STAGE_TRANSFER_BIT,
	END = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
};

namespace PipelineCreation
{

	class ShaderPipeline
	{
	public:
		ShaderPipeline(class Shader* Vertex, class Shader* Fragment);

		inline std::vector<VkPipelineShaderStageCreateInfo> GetShaderPipelineState() const { return mShaderPipeline; }

	private:
		std::vector<VkPipelineShaderStageCreateInfo> mShaderPipeline;

	};

	enum class DepthCompareOP
	{
		LESS = 0
	};

	class DepthStencilState
	{
	public:
		DepthStencilState(DepthCompareOP CompareOP = DepthCompareOP::LESS, bool EnableWrite = true, bool EnableTest = true, bool EnableStencil = false, float Min = 0.0f, float Max = 1.0f);

		inline VkPipelineDepthStencilStateCreateInfo GetDepthStencilState() const { return mDepthStencil; }

	private:
		VkPipelineDepthStencilStateCreateInfo mDepthStencil = {};

	};

	enum class AttachmentFlag : uint8_t
	{
		R = VK_COLOR_COMPONENT_R_BIT,
		G = VK_COLOR_COMPONENT_G_BIT,
		B = VK_COLOR_COMPONENT_B_BIT,
		A = VK_COLOR_COMPONENT_A_BIT
	};

	inline AttachmentFlag operator|(AttachmentFlag Left, AttachmentFlag Right)
	{
		return static_cast<AttachmentFlag>(static_cast<uint8_t>(Left) | static_cast<uint8_t>(Right));
	}

	class BlendDef
	{
	public:
		BlendDef(AttachmentFlag Components, bool EnableBlend = false);

		inline VkPipelineColorBlendAttachmentState GetState() const { return mState; }

	private:
		VkPipelineColorBlendAttachmentState mState = {};
	};

	class ColorBlendState
	{
	public:
		ColorBlendState(std::initializer_list<BlendDef> AttachmentDefinitions);

		inline VkPipelineColorBlendStateCreateInfo GetColorBlendState() const { return mColorBlendState; }

	private:
		VkPipelineColorBlendStateCreateInfo mColorBlendState = {};
		std::vector<VkPipelineColorBlendAttachmentState> mAttachemntStates;
	};

	enum class CullMode
	{
		FRONT = VK_CULL_MODE_FRONT_BIT,
		BACK = VK_CULL_MODE_BACK_BIT,
		FRONTBACK = VK_CULL_MODE_FRONT_AND_BACK
	};

	enum class FrontFace
	{
		CLOCKWISE = VK_FRONT_FACE_CLOCKWISE,
		COUNTERCLOCKWISE = VK_FRONT_FACE_COUNTER_CLOCKWISE
	};


	class RasterizationState
	{
	public:
		RasterizationState(CullMode Cull = CullMode::BACK, FrontFace FaceDir = FrontFace::CLOCKWISE);

		inline VkPipelineRasterizationStateCreateInfo GetRasterizationState() const { return mRasterizationState; }
	private:
		VkPipelineRasterizationStateCreateInfo mRasterizationState = {};
	};


	class MultisampleState
	{
	public:
		MultisampleState(bool Enable = false, uint8_t SampleCount = 1);

		inline VkPipelineMultisampleStateCreateInfo GetMultisampleState() const { return mMultisampleState; }
	private:
		VkPipelineMultisampleStateCreateInfo mMultisampleState = {};
	};


	class InputAssemblyState
	{
	public:
		InputAssemblyState();

		inline VkPipelineInputAssemblyStateCreateInfo GetInputAssemblyState() const { return mInputAssemblyState; }

	private:
		VkPipelineInputAssemblyStateCreateInfo mInputAssemblyState = {};
	};


	class VertexInputState
	{
	public:

		VertexInputState(Shader* VertexShader, std::initializer_list<VertexFormatDeclaration> VertexFormats);

		inline VkPipelineVertexInputStateCreateInfo GetVertexInputState() const { return mVertexInputState; }
	private:
		VkPipelineVertexInputStateCreateInfo mVertexInputState = {};
		std::vector<VkVertexInputBindingDescription> mInputBindings;
		std::vector<VkVertexInputAttributeDescription> mInputsAttributs;
	};


	class PipelineLayout
	{
	public:
		PipelineLayout(std::initializer_list<Shader*> Shaders);

		~PipelineLayout();

		inline VkPipelineLayout GetPipelineLayout() const { return mPipelineLayout; }
		inline VkDescriptorSetLayout GetDescriptorSetLayout() const { return mDescriptorLayout; }

	private:
		VkDescriptorSetLayout mDescriptorLayout = nullptr;
		VkPipelineLayout mPipelineLayout = nullptr;
	};

	class DynamicState
	{
	public:
		DynamicState();

		inline VkPipelineDynamicStateCreateInfo GetDynamicState() const { return mDynamicState; }

	private:
		VkPipelineDynamicStateCreateInfo mDynamicState = {};
		std::vector<VkDynamicState> mDynamicStates;

	};

}
