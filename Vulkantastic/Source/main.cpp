#include "Utilities/Engine.h"
#include <array>
#include "Renderer/vertex_definitions.h"
#include "Utilities/assert.h"
#include "Renderer/buffer.h"
#include "Renderer/image.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include "Renderer/command_buffer.h"
#include "Renderer/image_view.h"
#include "Renderer/sampler.h"
#include "Renderer/pipeline.h"

int32_t CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	Engine::Startup();

	Shader* ComputeShader = ShaderManager::Get().Find("first.comp");
	Shader* VertexShader = ShaderManager::Get().Find("first.vert");
	Shader* FragmentShader = ShaderManager::Get().Find("first.frag");

	// Start - Raw vulkan
	{
		auto Device = VulkanCore::Get().GetDevice()->GetDevice();
		VkExtent2D Extend = VulkanCore::Get().GetDevice()->GetSurfaceCapabilities().currentExtent;

		// Render Pass
		ColorAttachment Color = {};
		Color.EndLayout = ImageLayout::PRESENT_SRC;
		Color.Format = static_cast<ImageFormat>(VulkanCore::Get().GetSwapChain()->GetFormat().format);
		Color.Width = Extend.width;
		Color.Height = Extend.height;

		DepthAttachment Depth = {};

		RenderPass GraphicsRenderPass({ Color }, Depth);

		// Pipeline
		PipelineShaders Shaders{ VertexShader, FragmentShader };

		GraphicsPipeline<VertexDefinition::Simple, VertexDefinition::SimpleInstanced> Pipeline(GraphicsRenderPass, Shaders);

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

		// Compute shader
		uint32_t ComputeQueue = VulkanCore::Get().GetDevice()->GetQueuesIndicies().ComputeIndex;
		DescriptorManager ComputeDescManager({ ComputeShader });
		
		Buffer ComputeBuffer({ ComputeQueue }, BufferUsage::STORAGE | BufferUsage::UNIFORM, false, sizeof(float) * 5, nullptr);

		auto ComputeDescInst = ComputeDescManager.GetDescriptorInstance();
		ComputeDescInst->SetBuffer("", ComputeBuffer)->Update();

		VkPipelineLayout ComputeLayout;
		VkPipelineLayoutCreateInfo ComputeLayoutCreateInfo = {};
		ComputeLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		ComputeLayoutCreateInfo.setLayoutCount = 1;
		auto DescLayout = ComputeDescManager.GetLayout();
		ComputeLayoutCreateInfo.pSetLayouts = &DescLayout;

		vkCreatePipelineLayout(Device, &ComputeLayoutCreateInfo, nullptr, &ComputeLayout);

		VkComputePipelineCreateInfo ComputePipelineCreateInfo = {};
		ComputePipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
		ComputePipelineCreateInfo.layout = ComputeLayout;
		VkPipelineShaderStageCreateInfo ComputeShaderStage = {};
		ComputeShaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		ComputeShaderStage.module = ComputeShader->GetModule();
		ComputeShaderStage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
		ComputeShaderStage.pName = "main";
		ComputePipelineCreateInfo.stage = ComputeShaderStage;

		VkPipeline ComputePipeline = nullptr;
		vkCreateComputePipelines(Device, VK_NULL_HANDLE, 1, &ComputePipelineCreateInfo, nullptr, &ComputePipeline);

		// Compute
		CommandBuffer ComputeCB(ComputeQueue);
		ComputeCB.Begin(CBUsage::SIMULTANEOUS);
		vkCmdBindPipeline(ComputeCB.GetCommandBuffer(), VK_PIPELINE_BIND_POINT_COMPUTE, ComputePipeline);
		auto CSet = ComputeDescInst->GetSet();
		vkCmdBindDescriptorSets(ComputeCB.GetCommandBuffer(), VK_PIPELINE_BIND_POINT_COMPUTE, ComputeLayout, 0, 1, &CSet, 0, nullptr);
		vkCmdDispatch(ComputeCB.GetCommandBuffer(), 5, 1, 1);
		ComputeCB.End();

		// Prepare graphics descriptor
		auto DescInst = Pipeline.GetDescriptorManager()->GetDescriptorInstance();
		DescInst->SetBuffer("UBInstance", UniformBuffer)->SetImage("Image", View, SamplerInst)->SetBuffer("UBInstance2", ComputeBuffer)->Update();

		// Allocate and init command buffers
		std::vector<CommandBuffer*> CommandBuffers;

		const int32_t GraphicsIndex = VulkanCore::Get().GetDevice()->GetQueuesIndicies().GraphicsIndex;

		for (auto & Framebuffer : Framebuffers)
		{
			CommandBuffer* Cb = new CommandBuffer(GraphicsIndex);
			CommandBuffers.push_back(Cb);

			Cb->Begin(CBUsage::SIMULTANEOUS);

			const VkClearValue Clear[] = { { 0, 0, 0, 1 }, { 1.0f, 0.0f } };

			VkRenderPassBeginInfo RenderPassBeginInfo = {};
			RenderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			RenderPassBeginInfo.framebuffer = Framebuffer;
			RenderPassBeginInfo.renderPass = GraphicsRenderPass.GetRenderPass();
			RenderPassBeginInfo.renderArea.offset = { 0,0 };
			RenderPassBeginInfo.renderArea.extent = Extend;
			RenderPassBeginInfo.clearValueCount = 2;
			RenderPassBeginInfo.pClearValues = Clear;

			vkCmdBeginRenderPass(Cb->GetCommandBuffer(), &RenderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
			vkCmdBindPipeline(Cb->GetCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, Pipeline.GetPipeline());
			VkDeviceSize Offsets[] = { 0 };
			auto BufferTmp = VertexBuffer.GetBuffer();
			vkCmdBindVertexBuffers(Cb->GetCommandBuffer(), 0, 1, &BufferTmp, Offsets);
			auto InstBufferTmp = VertexBufferInst.GetBuffer();
			vkCmdBindVertexBuffers(Cb->GetCommandBuffer(), 1, 1, &InstBufferTmp, Offsets);
			vkCmdBindIndexBuffer(Cb->GetCommandBuffer(), IndexBuffer.GetBuffer(), 0, VK_INDEX_TYPE_UINT16);
			float VertPC[2] = { -0.2f,-0.3f };
			float FragPC[3] = { 0.0f,1.0f,0.0f };
			vkCmdPushConstants(Cb->GetCommandBuffer(), Pipeline.GetPipelineLayout()->GetPipelineLayout(), VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(float) * 4, sizeof(float) * 3, FragPC);
			vkCmdPushConstants(Cb->GetCommandBuffer(), Pipeline.GetPipelineLayout()->GetPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(float) * 2, VertPC);
			auto Viewports = Pipeline.GetViewportState()->GetViewports();
			vkCmdSetViewport(Cb->GetCommandBuffer(), 0, static_cast<uint32_t>(Viewports.size()), Viewports.data());
			auto Set = DescInst->GetSet();
			vkCmdBindDescriptorSets(Cb->GetCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, Pipeline.GetPipelineLayout()->GetPipelineLayout(), 0, 1, &Set, 0, nullptr);
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

		// Synchronization for compute
		VkSemaphore ComputeDone;
		vkCreateSemaphore(Device, &SemaphoreInfo, nullptr, &ComputeDone);

		int32_t CurrentFrame = 0;

		// End - Raw vulkan


		while (!Window::Get().ShouldWindowClose())
		{
			Window::Get().Update();

			vkWaitForFences(Device, 1, &FrameFence[CurrentFrame], VK_TRUE, std::numeric_limits<uint64_t>::max());
			vkResetFences(Device, 1, &FrameFence[CurrentFrame]);

			ComputeCB.Submit(false, { ComputeDone });

			// Graphics
			uint32_t ImageIndex;
			vkAcquireNextImageKHR(Device, VulkanCore::Get().GetSwapChain()->GetSwapChain(), std::numeric_limits<uint64_t>::max(), ImageReadyToDraw[CurrentFrame], VK_NULL_HANDLE, &ImageIndex);

			CommandBuffers[CurrentFrame]->Submit(FrameFence[CurrentFrame], { ImageReadyToPresent[CurrentFrame] }, { ImageReadyToDraw[CurrentFrame], ComputeDone }, { PipelineStage::COLOR_ATTACHMENT, PipelineStage::FRAGMENT });

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
