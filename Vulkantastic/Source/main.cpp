#include "Utilities/Engine.h"
#include <array>

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

		GraphicsPipeline<VertexDefinition::StaticMesh, VertexDefinition::SimpleInstanced> Pipeline(GraphicsRenderPass, Shaders);

		// Load mesh
		auto* Mesh = StaticMeshManager::Get().Find("test2");

		// Create vertex buffer
		uint32_t GraphicsQueueIndex = VulkanCore::Get().GetDevice()->GetQueuesIndicies().GraphicsIndex;

		// Instance data
		const std::vector<VertexDefinition::SimpleInstanced> VerticesInst = {
			{ { -1.0f, 1.0f, 1.0f } },
			{ { 1.0f, -1.0f, 1.0f } },
			{ { 1.0f, 1.0f, -1.0f } }
		};

		// Create vertex buffer for instance data
		Buffer VertexBufferInst({ GraphicsQueueIndex }, BufferUsage::VERTEX | BufferUsage::TRANSFER_DST, true, sizeof(VertexDefinition::SimpleInstanced) * VerticesInst.size(), VerticesInst.data());

		// Create image buffer
		int32_t Width, Height, Comp;
		stbi_uc* Pixels = stbi_load("Textures/test.jpg", &Width, &Height, &Comp, STBI_rgb_alpha);
		const int32_t ImageSize = Width * Height * STBI_rgb_alpha;

		ImageSettings Settings = {};
		Settings.Depth = 1;
		Settings.Height = Height;
		Settings.Width = Width;
		Settings.Format = ImageFormat::R8G8B8A8_SRGB;
		Settings.Type = ImageType::TWODIM;
		Settings.Mipmaps = true;

		Image ImageBuffer({ GraphicsQueueIndex }, ImageUsage::SAMPLED | ImageUsage::TRANSFER_DST | ImageUsage::TRANSFER_SRC, true, Settings, Pixels);
		ImageBuffer.ChangeLayout(ImageLayout::SHADER_READ);

		// Create image view
		ImageViewSettings ViewSettings = {};
		ViewSettings.MipMapLevelCount = ImageBuffer.GetMipMapsCount();
		ViewSettings.Format = ImageFormat::R8G8B8A8_SRGB;

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
		std::vector<std::unique_ptr<ImageView>> ImageViews;
		auto Images = VulkanCore::Get().GetSwapChain()->GetImages();
		ImageViews.reserve(Images.size());

		auto Format = VulkanCore::Get().GetSwapChain()->GetFormat().format;

		ImageViewSettings PresentationImageViewSettings = {};
		PresentationImageViewSettings.Format = static_cast<ImageFormat>(Format);

		for (auto& Image : Images)
		{
			ImageViews.push_back(std::make_unique<ImageView>(Image, PresentationImageViewSettings));
		}
		
		// Create framebuffers
		std::vector<std::unique_ptr<Framebuffer>> Framebuffers;
		Framebuffers.reserve(ImageViews.size());

		for (auto & View : ImageViews)
		{
			float Width = Extend.width;
			float Height = Extend.height;

			std::vector<ImageView*> Tmp = { View.get() , &DepthView };
			
			Framebuffers.push_back(std::make_unique<Framebuffer>(Tmp, GraphicsRenderPass, Width, Height));
		}

		// Compute shader
		ComputePipeline ComputePip(ComputeShader);

		uint32_t ComputeQueue = VulkanCore::Get().GetDevice()->GetQueuesIndicies().ComputeIndex;

		Buffer ComputeBuffer({ ComputeQueue, GraphicsQueueIndex }, BufferUsage::STORAGE, false, sizeof(float) * 5, nullptr);

		auto ComputeDescInst = ComputePip.GetDescriptorManager()->GetDescriptorInstance();
		ComputeDescInst->SetBuffer("", ComputeBuffer)->Update();

		CommandBuffer ComputeCB(ComputeQueue);
		ComputeCB.Begin(CBUsage::SIMULTANEOUS);
		vkCmdBindPipeline(ComputeCB.GetCommandBuffer(), VK_PIPELINE_BIND_POINT_COMPUTE, ComputePip.GetPipeline());
		auto CSet = ComputeDescInst->GetSet();
		vkCmdBindDescriptorSets(ComputeCB.GetCommandBuffer(), VK_PIPELINE_BIND_POINT_COMPUTE, ComputePip.GetPipelineLayout(), 0, 1, &CSet, 0, nullptr);
		vkCmdDispatch(ComputeCB.GetCommandBuffer(), 5, 1, 1);
		ComputeCB.End();

		// Update descriptor instance
		auto DescInst = Pipeline.GetDescriptorManager()->GetDescriptorInstance();	

		DescInst->SetImage("Image", View, SamplerInst)->SetBuffer("UBInstance2", ComputeBuffer);
		
		auto UBptr = DescInst->GetUniformBuffer("UBInstance");

		glm::vec2 UBData = { 0.1f, 0.1f };
		UBptr->Set("Offset", UBData);

		glm::vec2 VertPCData = { -0.2f,-0.3f };
		glm::vec3 FragPCData = { 0.0f,1.0f,0.0f };

		auto PCVertPtr = DescInst->GetPushConstantBuffer(ShaderType::VERTEX);
		PCVertPtr->Set("CustomOffset", VertPCData);

		
		auto Aspect = Extend.width / float(Extend.height);
		auto Projection = glm::perspective(3.14f / 4.0f, Aspect, 1.0f, 100.0f);
		auto Correction = glm::mat4(glm::vec4(1, 0, 0, 0), glm::vec4(0, -1, 0, 0), glm::vec4(0, 0, 1.0f / 2.0f, 1.0f / 2.0f), glm::vec4(0, 0, 0, 1));

		auto Camera = glm::lookAt(glm::vec3(3, 3, 3), glm::vec3(0,0,0) , glm::vec3(0, 1, 0));
		auto MVP = Correction * Projection * Camera * glm::translate(glm::mat4(1), glm::vec3(0.3f, 0.0f, 0.0f));

		PCVertPtr->Set("MVP", MVP);

		auto PCFragPtr = DescInst->GetPushConstantBuffer(ShaderType::FRAGMENT);
		PCFragPtr->Set("CustomColor", FragPCData);

		DescInst->Update();

		// Allocate and init command buffers
		std::vector<CommandBuffer*> CommandBuffers;

		const int32_t GraphicsIndex = VulkanCore::Get().GetDevice()->GetQueuesIndicies().GraphicsIndex;

		for (auto & FB : Framebuffers)
		{
			CommandBuffer* Cb = new CommandBuffer(GraphicsIndex);
			CommandBuffers.push_back(Cb);

			Cb->Begin(CBUsage::SIMULTANEOUS);

			const VkClearValue Clear[] = { { 0, 0, 0, 1 }, { 1.0f, 0.0f } };

			VkRenderPassBeginInfo RenderPassBeginInfo = {};
			RenderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			RenderPassBeginInfo.framebuffer = FB->GetFramebuffer();
			RenderPassBeginInfo.renderPass = GraphicsRenderPass.GetRenderPass();
			RenderPassBeginInfo.renderArea.offset = { 0,0 };
			RenderPassBeginInfo.renderArea.extent = Extend;
			RenderPassBeginInfo.clearValueCount = 2;
			RenderPassBeginInfo.pClearValues = Clear;

			vkCmdBeginRenderPass(Cb->GetCommandBuffer(), &RenderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
			vkCmdBindPipeline(Cb->GetCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, Pipeline.GetPipeline());

			VkDeviceSize Offsets[] = { 0 };
			auto BufferTmp = Mesh->GetVertexBuffer(0)->GetBuffer();
			vkCmdBindVertexBuffers(Cb->GetCommandBuffer(), 0, 1, &BufferTmp, Offsets);

			auto InstBufferTmp = VertexBufferInst.GetBuffer();
			vkCmdBindVertexBuffers(Cb->GetCommandBuffer(), 1, 1, &InstBufferTmp, Offsets);

			vkCmdBindIndexBuffer(Cb->GetCommandBuffer(), Mesh->GetIndexBuffer(0)->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);

			auto PCVertPtr = DescInst->GetPushConstantBuffer(ShaderType::VERTEX);
			vkCmdPushConstants(Cb->GetCommandBuffer(), Pipeline.GetPipelineLayout()->GetPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT, PCVertPtr->GetOffset(), PCVertPtr->GetSize(), PCVertPtr->GetBuffer());

			auto PCFragPtr = DescInst->GetPushConstantBuffer(ShaderType::FRAGMENT);
			vkCmdPushConstants(Cb->GetCommandBuffer(), Pipeline.GetPipelineLayout()->GetPipelineLayout(), VK_SHADER_STAGE_FRAGMENT_BIT, PCFragPtr->GetOffset(), PCFragPtr->GetSize(), PCFragPtr->GetBuffer());

			auto Viewports = Pipeline.GetViewportState()->GetViewports();
			vkCmdSetViewport(Cb->GetCommandBuffer(), 0, static_cast<uint32_t>(Viewports.size()), Viewports.data());
			auto Set = DescInst->GetSet();
			vkCmdBindDescriptorSets(Cb->GetCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, Pipeline.GetPipelineLayout()->GetPipelineLayout(), 0, 1, &Set, 0, nullptr);
			vkCmdDrawIndexed(Cb->GetCommandBuffer(), Mesh->GetIndiciesSize(0), 3, 0, 0, 0);
			vkCmdEndRenderPass(Cb->GetCommandBuffer());

			Cb->End();

		}

		// Create synchronization objects
		std::vector<Semaphore> ImageReadyToDraw(CommandBuffers.size());
		std::vector<Semaphore> ImageReadyToPresent(CommandBuffers.size());
		std::vector<Fence> FrameFence(CommandBuffers.size());

		Semaphore ComputeDone;

		// End - Raw vulkan


		while (!Window::Get().ShouldWindowClose())
		{
			Window::Get().Update();

			auto CurrentImageIndex = VulkanCore::Get().GetImageIndex();

			FrameFence[CurrentImageIndex].Wait();
			FrameFence[CurrentImageIndex].Reset();

			ComputeCB.Submit(false, { &ComputeDone });

			// Graphics
			uint32_t ImageIndex;
			vkAcquireNextImageKHR(Device, VulkanCore::Get().GetSwapChain()->GetSwapChain(), std::numeric_limits<uint64_t>::max(), ImageReadyToDraw[CurrentImageIndex].Get(), VK_NULL_HANDLE, &ImageIndex);

			CommandBuffers[CurrentImageIndex]->Submit(&FrameFence[CurrentImageIndex], { &ImageReadyToPresent[CurrentImageIndex] }, { &ImageReadyToDraw[CurrentImageIndex], &ComputeDone }, { PipelineStage::COLOR_ATTACHMENT, PipelineStage::FRAGMENT });

			VkPresentInfoKHR PresentInfo = {};
			PresentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
			PresentInfo.swapchainCount = 1;
			auto SwapChain = VulkanCore::Get().GetSwapChain()->GetSwapChain();
			PresentInfo.pSwapchains = &SwapChain;
			PresentInfo.waitSemaphoreCount = 1;
			PresentInfo.pWaitSemaphores = ImageReadyToPresent[CurrentImageIndex].GetPtr();
			PresentInfo.pImageIndices = &ImageIndex;

			vkQueuePresentKHR(VulkanCore::Get().GetDevice()->GetGraphicsQueue(), &PresentInfo);

			VulkanCore::Get().ProgessImageIndex();
		}

		vkDeviceWaitIdle(Device);

		for (auto& Cb : CommandBuffers)
		{
			delete Cb;
		}

	}

	Engine::Shutdown();
}
