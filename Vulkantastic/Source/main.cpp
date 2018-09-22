#include "Utilities/Engine.h"
#include <array>
#include "Renderer/vertex_definitions.h"
#include "Renderer/pipeline.h"
#include "Utilities/assert.h"
#include "Renderer/buffer.h"
#include "Renderer/image.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include "Renderer/command_buffer.h"
#include "Renderer/image_view.h"
#include "Renderer/sampler.h"
#include "Renderer/render_pass.h"

int32_t CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	Engine::Startup();

	Shader* VertexShader = ShaderManager::Get().Find("first.vert");
	Shader* FragmentShader = ShaderManager::Get().Find("first.frag");

	// Start - Raw vulkan
	{
		auto Device = VulkanCore::Get().GetDevice()->GetDevice();

		// Render Pass
		ColorAttachment Color = {};
		Color.EndLayout = ImageLayout::PRESENT_SRC;
		Color.Format = static_cast<ImageFormat>(VulkanCore::Get().GetSwapChain()->GetFormat().format);

		DepthAttachment Depth = {};

		RenderPass GraphicsRenderPass({ Color }, Depth);

		// Viewport
		VkExtent2D Extend = VulkanCore::Get().GetDevice()->GetSurfaceCapabilities().currentExtent;

		VkViewport Viewport = {};
		Viewport.width = static_cast<float>(Extend.width);
		Viewport.height = static_cast<float>(Extend.height);
		Viewport.maxDepth = 1.0f;
		Viewport.minDepth = 0.0f;

		VkRect2D Scissor = {};
		Scissor.extent = Extend;
		Scissor.offset = { 0,0 };

		VkPipelineViewportStateCreateInfo ViewportInfo = {};
		ViewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		ViewportInfo.scissorCount = 1;
		ViewportInfo.pScissors = &Scissor;
		ViewportInfo.viewportCount = 1;
		ViewportInfo.pViewports = &Viewport;

		// Pipeline
		VkPipeline GraphicsPipeline = nullptr;
		VkGraphicsPipelineCreateInfo GraphicsPipelineInfo = {};
		GraphicsPipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		GraphicsPipelineInfo.renderPass = GraphicsRenderPass.GetRenderPass();
		GraphicsPipelineInfo.pViewportState = &ViewportInfo;

		PipelineCreation::DynamicState Dynamic{};
		GraphicsPipelineInfo.pDynamicState = &Dynamic.GetDynamicState();

		PipelineCreation::PipelineLayout PipelineLayout({ VertexShader, FragmentShader });
		GraphicsPipelineInfo.layout = PipelineLayout.GetPipelineLayout();

		PipelineCreation::VertexInputState VertexInput(VertexShader, { VertexDefinition::Simple::VertexFormatInfo, VertexDefinition::SimpleInstanced::VertexFormatInfo });
		GraphicsPipelineInfo.pVertexInputState = &VertexInput.GetVertexInputState();

		PipelineCreation::ColorBlendState ColorBlend({ {PipelineCreation::AttachmentFlag::R | PipelineCreation::AttachmentFlag::G | PipelineCreation::AttachmentFlag::B | PipelineCreation::AttachmentFlag::A, false} });
		GraphicsPipelineInfo.pColorBlendState = &ColorBlend.GetColorBlendState();

		PipelineCreation::DepthStencilState DepthStencil{};
		GraphicsPipelineInfo.pDepthStencilState = &DepthStencil.GetDepthStencilState();

		PipelineCreation::MultisampleState Multisample{};
		GraphicsPipelineInfo.pMultisampleState = &Multisample.GetMultisampleState();

		PipelineCreation::RasterizationState Rasterization{};
		GraphicsPipelineInfo.pRasterizationState = &Rasterization.GetRasterizationState();

		PipelineCreation::InputAssemblyState InputAssembly{};
		GraphicsPipelineInfo.pInputAssemblyState = &InputAssembly.GetInputAssemblyState();

		PipelineCreation::ShaderPipeline ShaderPip(VertexShader, FragmentShader);
		auto ShaderPipelineInformation = ShaderPip.GetShaderPipelineState();
		GraphicsPipelineInfo.pStages = ShaderPipelineInformation.data();
		GraphicsPipelineInfo.stageCount = static_cast<uint32_t>(ShaderPipelineInformation.size());

		vkCreateGraphicsPipelines(Device, VK_NULL_HANDLE, 1, &GraphicsPipelineInfo, nullptr, &GraphicsPipeline);

		// Vertex buffer data
		const std::vector<VertexDefinition::Simple> Vertices = {
			{ { -0.5f, 0.5f },{ 0.0f, 0.0f, 1.0f }, {0.0f, 1.0f} }, // 0
			{ { 0.5f, 0.5f },{ 0.0f, 1.0f, 0.0f }, {1.0f, 1.0f} },  // 1
			{ { 0.5f, -0.5f },{ 1.0f, 0.0f, 0.0f }, {1.0f, 0.0f} }, // 2
			{ { -0.5f, -0.5f },{ 0.0f, 0.0f, 1.0f }, {0.0f, 0.0f} } // 3
		};

		// Create vertex buffer
		uint32_t GraphicsQueueIndex = VulkanCore::Get().GetDevice()->GetQueuesIndicies().GraphicsIndex;
		Buffer VertexBuffer({ GraphicsQueueIndex }, BufferUsage::VERTEX | BufferUsage::TRANSFER_DST, true, sizeof(VertexDefinition::Simple) * Vertices.size(), Vertices.data());

		// Instance data
		const std::vector<VertexDefinition::SimpleInstanced> VerticesInst = {
			{ { 0.0f, -0.5f } },
			{ { 0.5f, 0.5f } },
			{ { -0.5f, 0.5f } }
		};

		// Create vertex buffer for instance data
		Buffer VertexBufferInst({ GraphicsQueueIndex }, BufferUsage::VERTEX | BufferUsage::TRANSFER_DST, true, sizeof(VertexDefinition::SimpleInstanced) * VerticesInst.size(), VerticesInst.data());

		// Index buffer data
		const std::vector<int16_t> Indicies = {
			0,2,1,
			0,3,2
		};
		// Create index buffer
		Buffer IndexBuffer({ GraphicsQueueIndex }, BufferUsage::INDEX | BufferUsage::TRANSFER_DST, true, sizeof(int16_t) * Indicies.size(), Indicies.data());

		// Create uniform buffer
		Vector2D UBData = { 0.1f, 0.1f };

		Buffer UniformBuffer({ GraphicsQueueIndex }, BufferUsage::UNIFORM | BufferUsage::TRANSFER_DST, false, sizeof(Vector2D), &UBData);

		// Create image buffer
		int32_t Width, Height, Comp;
		stbi_uc* Pixels = stbi_load("Textures/test.jpg", &Width, &Height, &Comp, STBI_rgb_alpha);
		const int32_t ImageSize = Width * Height * STBI_rgb_alpha;

		ImageSettings Settings = {};
		Settings.Depth = 1;
		Settings.Height = Height;
		Settings.Width = Width;
		Settings.Format = ImageFormat::R8G8B8A8;
		Settings.Type = ImageType::TWODIM;
		Settings.Mipmaps = true;

		Image ImageBuffer({ GraphicsQueueIndex }, ImageUsage::SAMPLED | ImageUsage::TRANSFER_DST | ImageUsage::TRANSFER_SRC, true, Settings, Pixels);
		ImageBuffer.ChangeLayout(ImageLayout::SHADER_READ);

		// Create image view
		ImageViewSettings ViewSettings = {};
		ViewSettings.MipMapLevelCount = ImageBuffer.GetMipMapsCount();
		ViewSettings.Format = ImageFormat::R8G8B8A8;

		ImageView View(&ImageBuffer, ViewSettings);

		// Create depth buffer
		ImageSettings DepthSettings = {};
		DepthSettings.Depth = 1;
		DepthSettings.Height = Extend.height;
		DepthSettings.Width = Extend.width;
		DepthSettings.Format = ImageFormat::D24S8;
		DepthSettings.Type = ImageType::TWODIM;
		DepthSettings.Mipmaps = false;

		Image DepthBuffer({ GraphicsQueueIndex }, ImageUsage::DEPTH_ATTACHMENT, true, DepthSettings);
		DepthBuffer.ChangeLayout(ImageLayout::DEPTH_STENCIL_ATTACHMENT);

		// Create depth view
		ImageViewSettings DepthViewSettings = {};
		DepthViewSettings.Format = ImageFormat::D24S8;

		ImageView DepthView(&DepthBuffer, DepthViewSettings);

		// Create sampler
		SamplerSettings SamplerInstSettings = {};
		SamplerInstSettings.MaxAnisotropy = 16;

		Sampler SamplerInst(SamplerInstSettings);

		// Descriptor pool
		std::array<VkDescriptorPoolSize, 2> PoolSizes;

		PoolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		PoolSizes[0].descriptorCount = 1;

		PoolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		PoolSizes[1].descriptorCount = 1;

		VkDescriptorPoolCreateInfo CreateInfo = {};
		CreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		CreateInfo.poolSizeCount = static_cast<uint32_t>(PoolSizes.size());
		CreateInfo.pPoolSizes = PoolSizes.data();
		CreateInfo.maxSets = 1;

		VkDescriptorPool DescriptorPool;
		vkCreateDescriptorPool(Device, &CreateInfo, nullptr, &DescriptorPool);

		// Descriptor set
		VkDescriptorSetAllocateInfo AllocDescriptorSetInfo = {};
		AllocDescriptorSetInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		AllocDescriptorSetInfo.descriptorPool = DescriptorPool;
		AllocDescriptorSetInfo.descriptorSetCount = 1;
		auto DescLayout = PipelineLayout.GetDescriptorSetLayout();
		AllocDescriptorSetInfo.pSetLayouts = &DescLayout;

		VkDescriptorSet DescriptorSet;
		vkAllocateDescriptorSets(Device, &AllocDescriptorSetInfo, &DescriptorSet);

		// Update descriptor set
		VkDescriptorBufferInfo BufferInfo = {};
		BufferInfo.buffer = UniformBuffer.GetBuffer();
		BufferInfo.range = sizeof(Vector2D);

		VkDescriptorImageInfo ImageInfo = {};
		ImageInfo.imageLayout = static_cast<VkImageLayout>(ImageBuffer.GetCurrentLayout());
		ImageInfo.imageView = View.GetView();
		ImageInfo.sampler = SamplerInst.GetSampler();

		std::array<VkWriteDescriptorSet, 2> Sets = {};

		Sets[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		Sets[0].dstSet = DescriptorSet;
		Sets[0].dstBinding = 0;
		Sets[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		Sets[0].descriptorCount = 1;
		Sets[0].pBufferInfo = &BufferInfo;

		Sets[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		Sets[1].dstSet = DescriptorSet;
		Sets[1].dstBinding = 1;
		Sets[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		Sets[1].descriptorCount = 1;
		Sets[1].pImageInfo = &ImageInfo;

		vkUpdateDescriptorSets(Device, static_cast<uint32_t>(Sets.size()), Sets.data(), 0, nullptr);

		// Create image views from swap chain images

		std::vector<VkImageView> ImageViews;
		auto Images = VulkanCore::Get().GetSwapChain()->GetImages();
		ImageViews.reserve(Images.size());

		for (auto& Image : Images)
		{

			VkImageViewCreateInfo Info = {};
			Info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			Info.format = VulkanCore::Get().GetSwapChain()->GetFormat().format;
			Info.image = Image;
			Info.subresourceRange.baseMipLevel = 0;
			Info.subresourceRange.levelCount = 1;
			Info.subresourceRange.baseArrayLayer = 0;
			Info.subresourceRange.layerCount = 1;
			Info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			Info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			Info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			Info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			Info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
			Info.viewType = VK_IMAGE_VIEW_TYPE_2D;

			VkImageView View;
			vkCreateImageView(Device, &Info, nullptr, &View);

			ImageViews.push_back(View);

		}

		// Create framebuffers
		std::vector<VkFramebuffer> Framebuffers;
		Framebuffers.resize(ImageViews.size());
		for (int32_t i = 0; i < ImageViews.size(); ++i)
		{

			VkImageView Attachments[] = {
				ImageViews[i],
				DepthView.GetView()
			};

			VkFramebufferCreateInfo Info = {};
			Info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			Info.attachmentCount = 2;
			Info.pAttachments = Attachments;
			Info.layers = 1;
			Info.renderPass = GraphicsRenderPass.GetRenderPass();
			Info.height = Extend.height;
			Info.width = Extend.width;

			vkCreateFramebuffer(Device, &Info, nullptr, &Framebuffers[i]);

		}

		// Allocate and init command buffers
		std::vector<CommandBuffer*> CommandBuffers;

		const int32_t GraphicsIndex = VulkanCore::Get().GetDevice()->GetQueuesIndicies().GraphicsIndex;

		for (int32_t i = 0; i < Framebuffers.size(); ++i)
		{
			CommandBuffer* Cb = new CommandBuffer(GraphicsIndex);
			CommandBuffers.push_back(Cb);

			Cb->Begin(CBUsage::SIMULTANEOUS);

			const VkClearValue Clear[] = { { 0, 0, 0, 1 }, { 1.0f, 0.0f } };

			VkRenderPassBeginInfo RenderPassBeginInfo = {};
			RenderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			RenderPassBeginInfo.framebuffer = Framebuffers[i];
			RenderPassBeginInfo.renderPass = GraphicsRenderPass.GetRenderPass();
			RenderPassBeginInfo.renderArea.offset = { 0,0 };
			RenderPassBeginInfo.renderArea.extent = Extend;
			RenderPassBeginInfo.clearValueCount = 2;
			RenderPassBeginInfo.pClearValues = Clear;

			vkCmdBeginRenderPass(Cb->GetCommandBuffer(), &RenderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
			vkCmdBindPipeline(Cb->GetCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, GraphicsPipeline);
			VkDeviceSize Offsets[] = { 0 };
			auto BufferTmp = VertexBuffer.GetBuffer();
			vkCmdBindVertexBuffers(Cb->GetCommandBuffer(), 0, 1, &BufferTmp, Offsets);
			auto InstBufferTmp = VertexBufferInst.GetBuffer();
			vkCmdBindVertexBuffers(Cb->GetCommandBuffer(), 1, 1, &InstBufferTmp, Offsets);
			vkCmdBindIndexBuffer(Cb->GetCommandBuffer(), IndexBuffer.GetBuffer(), 0, VK_INDEX_TYPE_UINT16);
			vkCmdSetViewport(Cb->GetCommandBuffer(), 0, 1, &Viewport);
			vkCmdBindDescriptorSets(Cb->GetCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, PipelineLayout.GetPipelineLayout(), 0, 1, &DescriptorSet, 0, nullptr);
			vkCmdDrawIndexed(Cb->GetCommandBuffer(), Indicies.size(), 3, 0, 0, 0);
			vkCmdEndRenderPass(Cb->GetCommandBuffer());

			Cb->End();

		}

		// Create synchronization objects
		VkSemaphoreCreateInfo SemaphoreInfo = {};
		SemaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		std::vector<VkSemaphore> ImageReadyToDraw(CommandBuffers.size());
		std::vector<VkSemaphore> ImageReadyToPresent(CommandBuffers.size());
		std::vector<VkFence> FrameFence(CommandBuffers.size());

		VkFenceCreateInfo FenceInfo = {};
		FenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		FenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (int32_t i = 0; i < CommandBuffers.size(); ++i)
		{
			vkCreateSemaphore(Device, &SemaphoreInfo, nullptr, &ImageReadyToDraw[i]);
			vkCreateSemaphore(Device, &SemaphoreInfo, nullptr, &ImageReadyToPresent[i]);
			vkCreateFence(Device, &FenceInfo, nullptr, &FrameFence[i]);
		}

		int32_t CurrentFrame = 0;

		// End - Raw vulkan


		while (!Window::Get().ShouldWindowClose())
		{
			Window::Get().Update();

			vkWaitForFences(Device, 1, &FrameFence[CurrentFrame], VK_TRUE, std::numeric_limits<uint64_t>::max());
			vkResetFences(Device, 1, &FrameFence[CurrentFrame]);

			uint32_t ImageIndex;
			vkAcquireNextImageKHR(Device, VulkanCore::Get().GetSwapChain()->GetSwapChain(), std::numeric_limits<uint64_t>::max(), ImageReadyToDraw[CurrentFrame], VK_NULL_HANDLE, &ImageIndex);

			CommandBuffers[CurrentFrame]->Submit(FrameFence[CurrentFrame], { ImageReadyToPresent[CurrentFrame] }, { ImageReadyToDraw[CurrentFrame] }, { PipelineStage::COLOR_ATTACHMENT });

			VkPresentInfoKHR PresentInfo = {};
			PresentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
			PresentInfo.swapchainCount = 1;
			auto SwapChain = VulkanCore::Get().GetSwapChain()->GetSwapChain();
			PresentInfo.pSwapchains = &SwapChain;
			PresentInfo.waitSemaphoreCount = 1;
			PresentInfo.pWaitSemaphores = &ImageReadyToPresent[CurrentFrame];
			PresentInfo.pImageIndices = &ImageIndex;

			vkQueuePresentKHR(VulkanCore::Get().GetDevice()->GetGraphicsQueue(), &PresentInfo);

			CurrentFrame = (CurrentFrame + 1) % CommandBuffers.size();
		}

		for (auto& Cb : CommandBuffers)
		{
			delete Cb;
		}

	}

	Engine::Shutdown();
}
