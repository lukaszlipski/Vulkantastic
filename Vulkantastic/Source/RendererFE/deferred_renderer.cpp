#define NOMINMAX
#include <limits>
#include "deferred_renderer.h"
#include "renderer_commands.h"
#include "static_mesh.h"
#include "../Renderer/render_pass.h"
#include "../Renderer/core.h"
#include "../Renderer/swap_chain.h"
#include "../Renderer/pipeline_manager.h"


DeferredRenderer::~DeferredRenderer()
{

}

bool DeferredRenderer::Startup()
{
	const uint32_t GraphicsQueueIndex = VulkanCore::Get().GetDevice()->GetQueuesIndicies().GraphicsIndex;

	const VkExtent2D Extend = VulkanCore::Get().GetExtend();

	// Render pass for base pass
	ColorAttachment Color = {};
	Color.EndLayout = ImageLayout::PRESENT_SRC;
	Color.Format = static_cast<ImageFormat>(VulkanCore::Get().GetSwapChain()->GetFormat().format);
	Color.Width = static_cast<float>(Extend.width);
	Color.Height = static_cast<float>(Extend.height);

	DepthAttachment Depth = {};

	std::vector<ColorAttachment> ColorAttachments = { Color };

	mBasePassRenderPass = std::make_unique<RenderPass>(ColorAttachments, Depth);

	// Depth buffer for base pass
	ImageSettings DepthSettings = {};
	DepthSettings.Depth = 1;
	DepthSettings.Height = Extend.height;
	DepthSettings.Width = Extend.width;
	DepthSettings.Format = ImageFormat::D24S8;
	DepthSettings.Type = ImageType::TWODIM;
	DepthSettings.Mipmaps = false;

	std::vector<uint32_t> Queues = { GraphicsQueueIndex };
	mDepthBuffer = std::make_unique<Image>(Queues, ImageUsage::DEPTH_ATTACHMENT, true, DepthSettings);
	mDepthBuffer->ChangeLayout(ImageLayout::DEPTH_STENCIL_ATTACHMENT);

	ImageViewSettings DepthViewSettings = {};
	DepthViewSettings.Format = ImageFormat::D24S8;

	mDepthView = std::make_unique<ImageView>(mDepthBuffer.get(), DepthViewSettings);

	PrepareFramebuffers();
	PrepareSynchronizationPrimitives();

	return true;
}

bool DeferredRenderer::Shutdown()
{
	mCommandBuffer.reset();
	mBasePassRenderPass.reset();
	mDepthBuffer.reset();
	mDepthView.reset();
	mFrameFence.~Fence();

	for (auto& Semaphore : mImageReadyToDraw) {	Semaphore.~Semaphore();	}

	for (auto& Semaphore : mImageReadyToPresent) { Semaphore.~Semaphore(); }

	for (auto& View : mImageViews) { View.reset(); }

	for (auto& Framebuffer : mFramebuffers)	{ Framebuffer.reset(); }

	return true;
}

void DeferredRenderer::PrepareFramebuffers()
{
	const auto Format = VulkanCore::Get().GetSwapChain()->GetFormat().format;
	const VkExtent2D Extend = VulkanCore::Get().GetExtend();

	auto Images = VulkanCore::Get().GetSwapChain()->GetImages();
	mImageViews.reserve(Images.size());

	ImageViewSettings PresentationImageViewSettings = {};
	PresentationImageViewSettings.Format = static_cast<ImageFormat>(Format);

	for (auto& Image : Images)
	{
		mImageViews.push_back(std::make_unique<ImageView>(Image, PresentationImageViewSettings));
	}

	
	mFramebuffers.reserve(mImageViews.size());

	for (auto & View : mImageViews)
	{
		float Width = static_cast<float>(Extend.width);
		float Height = static_cast<float>(Extend.height);

		std::vector<ImageView*> Tmp = { View.get() , mDepthView.get() };

		mFramebuffers.push_back(std::make_unique<Framebuffer>(Tmp, *mBasePassRenderPass, Width, Height));
	}

}

void DeferredRenderer::PrepareSynchronizationPrimitives()
{
	const auto ImagesCount = VulkanCore::Get().GetSwapChain()->GetImagesCount();

	mImageReadyToDraw.resize(ImagesCount);
	mImageReadyToPresent.resize(ImagesCount);
}

void DeferredRenderer::Render(SceneData& Data)
{
	const uint32_t GraphicsQueueIndex = VulkanCore::Get().GetDevice()->GetQueuesIndicies().GraphicsIndex;
	const VkExtent2D Extend = VulkanCore::Get().GetExtend();
	auto Device = VulkanCore::Get().GetDevice()->GetDevice();
	auto CurrentImageIndex = VulkanCore::Get().GetImageIndex();

	mFrameFence.Wait();
	mFrameFence.Reset();

	uint32_t ImageIndex = AcquireNextImage(&mImageReadyToDraw[CurrentImageIndex]);

	struct RenderableData
	{
		StaticMesh* Mesh;
		glm::mat4 Transform;
		int32_t Id = -1;
	};

	std::map<PipelineManager::KeyType, std::vector<RenderableData>> PartitionedRendererData;

	for (StaticMeshComponent* Mesh : Data.StaticMeshComponents)
	{
		auto CurrentMesh = Mesh->GetMesh();

		if(!CurrentMesh) { continue;}

		for (int32_t i = 0; i < CurrentMesh->GetMaterialsCount(); ++i)
		{
			const StaticSurfaceMaterial* Material = CurrentMesh->GetMaterial(i);
			const PipelineManager::KeyType Key = Material->GetDescriptorInstance()->GetPipelineKey();
			
			RenderableData NewData = {};
			NewData.Id = i;
			NewData.Mesh = CurrentMesh;
			NewData.Transform = Mesh->GetTransform();

			PartitionedRendererData[Key].emplace_back(std::move(NewData));
		}
	}

	mCommandBuffer = std::make_unique<CommandBuffer>(GraphicsQueueIndex);

	mCommandBuffer->Begin(CBUsage::ONE_TIME);

	const std::vector<VkClearValue> ClearColors = { { 0, 0, 0, 1 }, { 1.0f, 0.0f } };
	CmdBeginRenderPass(mCommandBuffer.get(), mFramebuffers[ImageIndex].get(), mBasePassRenderPass.get(), ClearColors, Extend);
	
	for (auto& PartitionedData : PartitionedRendererData)
	{
		const PipelineManager::KeyType PipelineKey = PartitionedData.first;
		std::vector<RenderableData>& RenderableDataList = PartitionedData.second;

		IGraphicsPipeline* Pipeline = PipelineManager::Get().GetPipelineByKey(PipelineKey);

		CmdBindGraphicsPipeline(mCommandBuffer.get(), Pipeline);

		for (RenderableData& DataToRender : RenderableDataList)
		{
			StaticMesh* const Mesh = DataToRender.Mesh;
			const int32_t Id = DataToRender.Id;
			DescriptorInst* DescInst = Mesh->GetMaterial(Id)->GetDescriptorInstance();

			const float Aspect = Extend.width / float(Extend.height);
			const glm::mat4 Projection = glm::perspective(3.14f / 4.0f, Aspect, 1.0f, 100.0f);
			const glm::mat4 Correction = glm::mat4(glm::vec4(1, 0, 0, 0), glm::vec4(0, -1, 0, 0), glm::vec4(0, 0, 1.0f / 2.0f, 1.0f / 2.0f), glm::vec4(0, 0, 0, 1));

			glm::mat4 Camera = glm::lookAt(Data.CameraPosition, Data.CameraPosition + Data.CameraForward, glm::vec3(0, 1, 0));
			glm::mat4 MVP = Correction * Projection * Camera * DataToRender.Transform; // #TODO: Add world transform

			Mesh->GetMaterial(Id)->SetMVP(MVP);

			Mesh->GetMaterial(Id)->Update();

			CmdBindVertexAndIndexBuffer(mCommandBuffer.get(), Mesh->GetVertexBuffer(Id), Mesh->GetIndexBuffer(Id));
			CmdUpdateDescriptorData(mCommandBuffer.get(), DescInst, Pipeline);
			CmdSetViewports(mCommandBuffer.get(), Pipeline);
			CmdDrawIndexed(mCommandBuffer.get(), Mesh->GetIndiciesSize(Id));

		}
	}

	CmdEndRenderPass(mCommandBuffer.get());

	mCommandBuffer->End();

	mCommandBuffer->Submit(&mFrameFence, { &mImageReadyToPresent[CurrentImageIndex] }, { &mImageReadyToDraw[CurrentImageIndex] }, { PipelineStage::COLOR_ATTACHMENT, PipelineStage::FRAGMENT });

	QueuePresent(ImageIndex, &mImageReadyToPresent[CurrentImageIndex]);
}
