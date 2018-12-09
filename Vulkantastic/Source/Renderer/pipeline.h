#pragma once
#include "vulkan/vulkan_core.h"
#include "render_pass.h"
#include <memory>
#include "descriptor_manager.h"

enum class PipelineType : uint8_t
{
	GRAPHICS = 0,
	COMPUTE
};

class PipelineShaders
{
public:
	PipelineShaders(std::initializer_list<Shader*> Shaders);

	inline Shader* GetVertexShader() const { return mVertex; }
	inline Shader* GetFragmentShader() const { return mFragment; }
	inline std::vector<Shader*> GetShaders() const { return mShaders; }

private:
	Shader* mVertex = nullptr;
	Shader* mFragment = nullptr;
	std::vector<Shader*> mShaders;

};

class IPipeline
{
public:
	virtual ~IPipeline() = default;

	virtual VkPipeline GetPipeline() const = 0;
	virtual DescriptorManager* GetDescriptorManager() = 0;
	virtual VkPipelineLayout GetPipelineLayout() = 0;

};

class IGraphicsPipeline : public IPipeline
{
public:
	
	virtual std::vector<VkViewport> GetViewports() = 0;
	
};

class ComputePipeline : public IPipeline
{
public:
	ComputePipeline(Shader* ComputeShader);
	~ComputePipeline();

	ComputePipeline(const ComputePipeline& Rhs) = delete;
	ComputePipeline& operator=(const ComputePipeline& Rhs) = delete;

	ComputePipeline(ComputePipeline&& Rhs) = delete;
	ComputePipeline& operator=(ComputePipeline&& Rhs) = delete;

	virtual VkPipeline GetPipeline() const override { return mPipeline; }
	virtual DescriptorManager* GetDescriptorManager() override { return mDescriptorManager.get(); }
	virtual VkPipelineLayout GetPipelineLayout() override { return mPipelineLayout; }

private:
	VkPipeline mPipeline = nullptr;
	VkPipelineLayout mPipelineLayout = nullptr;
	std::unique_ptr<DescriptorManager> mDescriptorManager;

};


template<typename ...VertexDef>
class GraphicsPipeline : public IGraphicsPipeline
{
public:
	GraphicsPipeline(const RenderPass& GraphicsRenderPass, PipelineShaders Shaders);
	~GraphicsPipeline();

	GraphicsPipeline(const GraphicsPipeline& Rhs) = delete;
	GraphicsPipeline& operator=(const GraphicsPipeline& Rhs) = delete;

	GraphicsPipeline(GraphicsPipeline&& Rhs) = delete;
	GraphicsPipeline& operator=(GraphicsPipeline&& Rhs) = delete;

	virtual VkPipeline GetPipeline() const override { return mPipeline; }
	virtual DescriptorManager* GetDescriptorManager() override { return mDescriptorManager.get(); }
	virtual VkPipelineLayout GetPipelineLayout() override { return mPipelineLayout->GetPipelineLayout(); }
	virtual std::vector<VkViewport> GetViewports() override { return mViewportState->GetViewports(); }

private:
	VkPipeline mPipeline = nullptr;
	std::unique_ptr<PipelineCreation::PipelineLayout> mPipelineLayout;
	std::unique_ptr<PipelineCreation::ViewportState> mViewportState;
	std::unique_ptr<DescriptorManager> mDescriptorManager;
	
};

template<typename ...VertexDef>
GraphicsPipeline<VertexDef...>::GraphicsPipeline(const RenderPass& GraphicsRenderPass, PipelineShaders Shaders)
{
	auto Device = VulkanCore::Get().GetDevice()->GetDevice();

	VkGraphicsPipelineCreateInfo GraphicsPipelineInfo = {};
	GraphicsPipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	GraphicsPipelineInfo.renderPass = GraphicsRenderPass.GetRenderPass();
	
	const auto& ColorAttachments = GraphicsRenderPass.GetColorAttachments();
	std::vector<PipelineCreation::ViewportSize> Viewports;
	Viewports.reserve(ColorAttachments.size());

	for (auto& Attachment : ColorAttachments)
	{
		Viewports.push_back({ Attachment.Width, Attachment.Height });
	}

	mViewportState = std::make_unique<PipelineCreation::ViewportState>(Viewports);
	GraphicsPipelineInfo.pViewportState = mViewportState->GetViewportState();
	
	PipelineCreation::DynamicState Dynamic{};
	GraphicsPipelineInfo.pDynamicState = Dynamic.GetDynamicState();

	auto ShadersList = { Shaders.GetVertexShader(), Shaders.GetFragmentShader() };
	mDescriptorManager = std::make_unique<DescriptorManager>(ShadersList);

	mPipelineLayout = std::make_unique<PipelineCreation::PipelineLayout>(mDescriptorManager.get());
	GraphicsPipelineInfo.layout = mPipelineLayout->GetPipelineLayout();

	PipelineCreation::VertexInputState VertexInput(Shaders.GetVertexShader(), { (VertexDef::VertexFormatInfo)... });
	GraphicsPipelineInfo.pVertexInputState = VertexInput.GetVertexInputState();

	std::vector<PipelineCreation::BlendDef> ColorBlends;
	ColorBlends.reserve(ColorAttachments.size());
	for (auto& Attachment : ColorAttachments)
	{
		const auto NumComp = Image::GetNumComponentsByFormat(Attachment.Format);

		auto Flags = static_cast<PipelineCreation::AttachmentFlag>(0);

		for (uint8_t i = 1; i <= NumComp; ++i)
		{
			Flags = Flags | static_cast<PipelineCreation::AttachmentFlag>(i);
		}

		ColorBlends.emplace_back( Flags, Attachment.Blend );
	}

	PipelineCreation::ColorBlendState ColorBlend(ColorBlends);
	GraphicsPipelineInfo.pColorBlendState = ColorBlend.GetColorBlendState();

	PipelineCreation::DepthStencilState DepthStencil{};
	GraphicsPipelineInfo.pDepthStencilState = DepthStencil.GetDepthStencilState();

	PipelineCreation::MultisampleState Multisample{};
	GraphicsPipelineInfo.pMultisampleState = Multisample.GetMultisampleState();

	PipelineCreation::RasterizationState Rasterization{};
	GraphicsPipelineInfo.pRasterizationState = Rasterization.GetRasterizationState();

	PipelineCreation::InputAssemblyState InputAssembly{};
	GraphicsPipelineInfo.pInputAssemblyState = InputAssembly.GetInputAssemblyState();

	PipelineCreation::ShaderPipeline ShaderPip(Shaders.GetVertexShader(), Shaders.GetFragmentShader());
	auto ShaderPipelineInformation = ShaderPip.GetShaderPipelineState();
	GraphicsPipelineInfo.pStages = ShaderPipelineInformation.data();
	GraphicsPipelineInfo.stageCount = static_cast<uint32_t>(ShaderPipelineInformation.size());

	vkCreateGraphicsPipelines(Device, VK_NULL_HANDLE, 1, &GraphicsPipelineInfo, nullptr, &mPipeline);
}


template<typename ...VertexDef>
GraphicsPipeline<VertexDef...>::~GraphicsPipeline()
{
	if (mPipeline)
	{
		auto Device = VulkanCore::Get().GetDevice()->GetDevice();

		vkDestroyPipeline(Device, mPipeline, nullptr);
	}
}
