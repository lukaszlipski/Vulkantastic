#pragma once
#include "core.h"
#include <vector>
#include "vertex_definitions_inc.h"

class Shader;
class DescriptorManager;

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

enum class BlendMode : uint8_t
{
	Opaque = 0,
	Translucent,
	Additive
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

		inline VkPipelineDepthStencilStateCreateInfo* GetDepthStencilState() { return &mDepthStencil; }

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
		BlendDef(AttachmentFlag Components, BlendMode Mode = BlendMode::Opaque);

		inline VkPipelineColorBlendAttachmentState GetState() const { return mState; }

	private:
		VkPipelineColorBlendAttachmentState mState = {};
	};

	class ColorBlendState
	{
	public:
		ColorBlendState(const std::vector<BlendDef>& AttachmentDefinitions);

		inline VkPipelineColorBlendStateCreateInfo* GetColorBlendState() { return &mColorBlendState; }

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
		RasterizationState(CullMode Cull = CullMode::BACK, FrontFace FaceDir = FrontFace::COUNTERCLOCKWISE);

		inline VkPipelineRasterizationStateCreateInfo* GetRasterizationState() { return &mRasterizationState; }
	private:
		VkPipelineRasterizationStateCreateInfo mRasterizationState = {};
	};


	class MultisampleState
	{
	public:
		MultisampleState(bool Enable = false, uint8_t SampleCount = 1);

		inline VkPipelineMultisampleStateCreateInfo* GetMultisampleState() { return &mMultisampleState; }
	private:
		VkPipelineMultisampleStateCreateInfo mMultisampleState = {};
	};


	class InputAssemblyState
	{
	public:
		InputAssemblyState();

		inline VkPipelineInputAssemblyStateCreateInfo* GetInputAssemblyState() { return &mInputAssemblyState; }

	private:
		VkPipelineInputAssemblyStateCreateInfo mInputAssemblyState = {};
	};


	class VertexInputState
	{
	public:

		VertexInputState(Shader* VertexShader, std::initializer_list<VertexFormatDeclaration> VertexFormats);

		inline VkPipelineVertexInputStateCreateInfo* GetVertexInputState() { return &mVertexInputState; }
	private:
		VkPipelineVertexInputStateCreateInfo mVertexInputState = {};
		std::vector<VkVertexInputBindingDescription> mInputBindings;
		std::vector<VkVertexInputAttributeDescription> mInputsAttributs;
	};

	struct ViewportSize
	{
		float Width = 0;
		float Height = 0;
	};

	class ViewportState
	{
	public:
		ViewportState(const std::vector<ViewportSize>& Sizes);

		inline VkPipelineViewportStateCreateInfo* GetViewportState() { return &mViewportState; }
		inline std::vector<VkViewport> GetViewports() const { return mViewports; }
		inline std::vector<VkRect2D> GetScissors() const { return mScissors; }

	private:
		VkPipelineViewportStateCreateInfo mViewportState = {};
		std::vector<VkViewport> mViewports;
		std::vector<VkRect2D> mScissors;

	};

	class PipelineLayout
	{
	public:
		PipelineLayout(DescriptorManager* DescManager);

		~PipelineLayout();

		inline VkPipelineLayout GetPipelineLayout() const { return mPipelineLayout; }

	private:
		VkPipelineLayout mPipelineLayout = nullptr;
	};

	class DynamicState
	{
	public:
		DynamicState();

		inline VkPipelineDynamicStateCreateInfo* GetDynamicState() { return &mDynamicState; }

	private:
		VkPipelineDynamicStateCreateInfo mDynamicState = {};
		std::vector<VkDynamicState> mDynamicStates;

	};

}
