#define NOMINMAX
#include <limits>
#include "deferred_renderer.h"
#include "static_mesh.h"
#include "../Renderer/render_pass.h"
#include "../Renderer/core.h"
#include "../Renderer/swap_chain.h"
#include "../Renderer/pipeline_manager.h"
#include "../Renderer/vertex_definitions.h"
#include "../Renderer/descriptor_manager.h"
#include "../Renderer/buffer.h"
#include "../Renderer/renderer_commands.h"


DeferredRenderer::~DeferredRenderer()
{

}

bool DeferredRenderer::Startup()
{
	const uint32_t GraphicsQueueIndex = VulkanCore::Get().GetDevice()->GetQueuesIndicies().GraphicsIndex;

	const VkExtent2D Extend = VulkanCore::Get().GetExtend();

	// Render pass for base pass
	{
		ColorAttachment Color = {};
		Color.EndLayout = ImageLayout::COLOR_ATTACHMENT;
		Color.Format = ImageFormat::R8G8B8A8;
		Color.Width = static_cast<float>(Extend.width);
		Color.Height = static_cast<float>(Extend.height);

		ColorAttachment Normal = {};
		Normal.EndLayout = ImageLayout::COLOR_ATTACHMENT;
		Normal.Format = ImageFormat::R8G8B8A8;
		Normal.Width = static_cast<float>(Extend.width);
		Normal.Height = static_cast<float>(Extend.height);

		ColorAttachment Position = {};
		Position.EndLayout = ImageLayout::COLOR_ATTACHMENT;
		Position.Format = ImageFormat::R8G8B8A8;
		Position.Width = static_cast<float>(Extend.width);
		Position.Height = static_cast<float>(Extend.height);

		DepthAttachment Depth = {};

		std::vector<ColorAttachment> ColorAttachments = { Color, Normal, Position };

		mBasePassRenderPass = std::make_unique<RenderPass>(ColorAttachments, Depth);
	}

	// Depth buffer for base pass
	{
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
	}

	// GBuffer setup
	{
		ImageSettings BasePassColorSettings = {};
		BasePassColorSettings.Depth = 1;
		BasePassColorSettings.Height = Extend.height;
		BasePassColorSettings.Width = Extend.width;
		BasePassColorSettings.Format = ImageFormat::R8G8B8A8;
		BasePassColorSettings.Type = ImageType::TWODIM;
		BasePassColorSettings.Mipmaps = false;

		std::vector<uint32_t> Queues = { GraphicsQueueIndex };
		mColorBuffer = std::make_unique<Image>(Queues, ImageUsage::COLOR_ATTACHMENT | ImageUsage::SAMPLED, true, BasePassColorSettings);
		mColorBuffer->ChangeLayout(ImageLayout::COLOR_ATTACHMENT);

		ImageViewSettings BasePassColorViewSettings = {};
		BasePassColorViewSettings.Format = ImageFormat::R8G8B8A8;

		mColorView = std::make_unique<ImageView>(mColorBuffer.get(), BasePassColorViewSettings);

		ImageSettings BasePassNormalSettings = {};
		BasePassNormalSettings.Depth = 1;
		BasePassNormalSettings.Height = Extend.height;
		BasePassNormalSettings.Width = Extend.width;
		BasePassNormalSettings.Format = ImageFormat::R8G8B8A8;
		BasePassNormalSettings.Type = ImageType::TWODIM;
		BasePassNormalSettings.Mipmaps = false;

		mNormalBuffer = std::make_unique<Image>(Queues, ImageUsage::COLOR_ATTACHMENT | ImageUsage::SAMPLED, true, BasePassNormalSettings);
		mNormalBuffer->ChangeLayout(ImageLayout::COLOR_ATTACHMENT);

		ImageViewSettings BasePassNormalViewSettings = {};
		BasePassNormalViewSettings.Format = ImageFormat::R8G8B8A8;

		mNormalView = std::make_unique<ImageView>(mNormalBuffer.get(), BasePassNormalViewSettings);

		ImageSettings BasePassPositionSettings = {};
		BasePassPositionSettings.Depth = 1;
		BasePassPositionSettings.Height = Extend.height;
		BasePassPositionSettings.Width = Extend.width;
		BasePassPositionSettings.Format = ImageFormat::R8G8B8A8;
		BasePassPositionSettings.Type = ImageType::TWODIM;
		BasePassPositionSettings.Mipmaps = false;

		mPositionBuffer = std::make_unique<Image>(Queues, ImageUsage::COLOR_ATTACHMENT | ImageUsage::SAMPLED, true, BasePassPositionSettings);
		mPositionBuffer->ChangeLayout(ImageLayout::COLOR_ATTACHMENT);

		ImageViewSettings BasePassPositionViewSettings = {};
		BasePassPositionViewSettings.Format = ImageFormat::R8G8B8A8;

		mPositionView = std::make_unique<ImageView>(mPositionBuffer.get(), BasePassPositionViewSettings);
		
	}

	// Render pass for light pass
	{
		ColorAttachment Color = {};
		Color.EndLayout = ImageLayout::COLOR_ATTACHMENT;
		Color.Format = ImageFormat::R8G8B8A8;
		Color.Width = static_cast<float>(Extend.width);
		Color.Height = static_cast<float>(Extend.height);

		std::vector<ColorAttachment> ColorAttachments = { Color };

		mLightPassRenderPass = std::make_unique<RenderPass>(ColorAttachments);
	}

	// Light pass' descriptors
	{
		Shader* VertexShader = ShaderManager::Get().Find("LightPass.vert");
		Shader* FragmentShader = ShaderManager::Get().Find("DirectionalLightPass.frag");

		Assert(VertexShader && FragmentShader);

		PipelineShaders Shaders{ VertexShader, FragmentShader };

		mDirectionalLightPassDescriporInst = PipelineManager::Get().GetDescriptorInstance<VertexDefinition::SimpleScreen>(*mLightPassRenderPass, Shaders);

	}

	// Scene texture
	{
		ImageSettings SceneSettings = {};
		SceneSettings.Depth = 1;
		SceneSettings.Height = Extend.height;
		SceneSettings.Width = Extend.width;
		SceneSettings.Format = ImageFormat::R8G8B8A8;
		SceneSettings.Type = ImageType::TWODIM;
		SceneSettings.Mipmaps = false;

		std::vector<uint32_t> Queues = { GraphicsQueueIndex };
		mSceneBuffer = std::make_unique<Image>(Queues, ImageUsage::COLOR_ATTACHMENT | ImageUsage::SAMPLED, true, SceneSettings);
		mSceneBuffer->ChangeLayout(ImageLayout::COLOR_ATTACHMENT);

		ImageViewSettings SceneViewSettings = {};
		SceneViewSettings.Format = ImageFormat::R8G8B8A8;

		mSceneView = std::make_unique<ImageView>(mSceneBuffer.get(), SceneViewSettings);
		
	}

	// Render pass for color to a screen
	{
		ColorAttachment Color = {};
		Color.EndLayout = ImageLayout::PRESENT_SRC;
		Color.Format = static_cast<ImageFormat>(VulkanCore::Get().GetSwapChain()->GetFormat().format);
		Color.Width = static_cast<float>(Extend.width);
		Color.Height = static_cast<float>(Extend.height);

		std::vector<ColorAttachment> ColorAttachments = { Color };

		mScreenRenderPass = std::make_unique<RenderPass>(ColorAttachments);

	}

	// Prepare screen's mesh and descriptor
	{
		Shader* VertexShader = ShaderManager::Get().Find("Screen.vert");
		Shader* FragmentShader = ShaderManager::Get().Find("Screen.frag");

		Assert(VertexShader && FragmentShader);

		PipelineShaders Shaders{ VertexShader, FragmentShader };
		
		mScreenDescriporInst = PipelineManager::Get().GetDescriptorInstance<VertexDefinition::SimpleScreen>(*mScreenRenderPass, Shaders);

		std::vector<uint32_t> QueueIndicies = { GraphicsQueueIndex };

		VertexDefinition::SimpleScreen LeftBottom = { {-1.0f, -1.0f}, {0.0f,0.0f} };
		VertexDefinition::SimpleScreen RightBottom = { {1.0f, -1.0f}, {1.0f,0.0f} };
		VertexDefinition::SimpleScreen LeftTop = { {-1.0f, 1.0f}, {0.0f, 1.0f} };
		VertexDefinition::SimpleScreen RightTop = { {1.0f, 1.0f}, {1.0f, 1.0f} };

		std::vector<VertexDefinition::SimpleScreen> Verticies = { LeftBottom, LeftTop, RightBottom, RightBottom, LeftTop, RightTop };

		mScreenVertexBuffer = std::make_unique<Buffer>(QueueIndicies, BufferUsage::VERTEX | BufferUsage::TRANSFER_DST, true, static_cast<uint32_t>(sizeof(VertexDefinition::SimpleScreen) * Verticies.size()), Verticies.data());
	}

	PrepareFramebuffers();
	PrepareSynchronizationPrimitives();

	return true;
}

bool DeferredRenderer::Shutdown()
{
	mBasePassCommandBuffer.reset();
	mLightPassCommandBuffer.reset();
	mScreenCommandBuffer.reset();

	mBasePassRenderPass.reset();
	mLightPassRenderPass.reset();
	mScreenRenderPass.reset();

	mDepthBuffer.reset();
	mDepthView.reset();

	mColorBuffer.reset();
	mColorView.reset();
	mNormalBuffer.reset();
	mNormalView.reset();
	mPositionBuffer.reset();
	mPositionView.reset();

	mSceneBuffer.reset();
	mSceneView.reset();

	mScreenVertexBuffer.reset();
	mScreenDescriporInst.reset();

	mBasePassFramebuffer.reset();
	mLightPassFramebuffer.reset();

	mFrameFence.reset();
	mBasePassReady.reset();
	mLightPassReady.reset();

	for (auto& Semaphore : mImageReadyToDraw) {	Semaphore.reset();	}
	for (auto& Semaphore : mImageReadyToPresent) { Semaphore.reset(); }
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

	// Base pass
	{
		float Width = static_cast<float>(Extend.width);
		float Height = static_cast<float>(Extend.height);

		std::vector<ImageView*> Tmp = { mColorView.get(), mNormalView.get(), mPositionView.get() , mDepthView.get() };

		mBasePassFramebuffer = std::make_unique<Framebuffer>(Tmp, *mBasePassRenderPass, Width, Height);
	}

	// Light pass
	{
		float Width = static_cast<float>(Extend.width);
		float Height = static_cast<float>(Extend.height);

		std::vector<ImageView*> Tmp = { mSceneView.get() };

		mLightPassFramebuffer = std::make_unique<Framebuffer>(Tmp, *mLightPassRenderPass, Width, Height);
	}

	// Screen
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

		std::vector<ImageView*> Tmp = { View.get() };

		mFramebuffers.push_back(std::make_unique<Framebuffer>(Tmp, *mScreenRenderPass, Width, Height));
	}

}

void DeferredRenderer::PrepareSynchronizationPrimitives()
{
	const auto ImagesCount = VulkanCore::Get().GetSwapChain()->GetImagesCount();

	mImageReadyToDraw.resize(ImagesCount);
	mImageReadyToPresent.resize(ImagesCount);

	for (uint32_t i = 0; i < ImagesCount; ++i)
	{
		mImageReadyToDraw[i] = std::make_unique<Semaphore>();
		mImageReadyToPresent[i] = std::make_unique<Semaphore>();
	}

	mFrameFence = std::make_unique<Fence>();
	mBasePassReady = std::make_unique<Semaphore>();
	mLightPassReady = std::make_unique<Semaphore>();
}

void DeferredRenderer::Render(SceneData& Data)
{
	const uint32_t GraphicsQueueIndex = VulkanCore::Get().GetDevice()->GetQueuesIndicies().GraphicsIndex;
	const VkExtent2D Extend = VulkanCore::Get().GetExtend();
	auto Device = VulkanCore::Get().GetDevice()->GetDevice();
	auto CurrentImageIndex = VulkanCore::Get().GetImageIndex();

	mFrameFence->Wait();
	mFrameFence->Reset();

	uint32_t ImageIndex = AcquireNextImage(mImageReadyToDraw[CurrentImageIndex].get());

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



	Image::ChangeMultipleLayouts(	{ mColorBuffer.get(),				mNormalBuffer.get(),			mPositionBuffer.get() }, 
									{ ImageLayout::COLOR_ATTACHMENT,	ImageLayout::COLOR_ATTACHMENT,	ImageLayout::COLOR_ATTACHMENT });


	// Base pass
	mBasePassCommandBuffer = std::make_unique<CommandBuffer>(GraphicsQueueIndex);

	mBasePassCommandBuffer->Begin(CBUsage::ONE_TIME);

	const std::vector<VkClearValue> ClearColors = { { 0, 0, 0, 1 }, { 0, 0, 0, 1 }, { 0, 0, 0, 1 }, { 1.0f, 0.0f } };
	Cmd::BeginRenderPass(mBasePassCommandBuffer.get(), mBasePassFramebuffer.get(), mBasePassRenderPass.get(), ClearColors, Extend);
	
	for (auto& PartitionedData : PartitionedRendererData)
	{
		const PipelineManager::KeyType PipelineKey = PartitionedData.first;
		std::vector<RenderableData>& RenderableDataList = PartitionedData.second;

		IGraphicsPipeline* Pipeline = PipelineManager::Get().GetPipelineByKey(PipelineKey);

		Cmd::BindGraphicsPipeline(mBasePassCommandBuffer.get(), Pipeline);

		for (RenderableData& DataToRender : RenderableDataList)
		{
			StaticMesh* const Mesh = DataToRender.Mesh;
			const int32_t Id = DataToRender.Id;
			DescriptorInst* DescInst = Mesh->GetMaterial(Id)->GetDescriptorInstance();

			const float Aspect = Extend.width / float(Extend.height);
			const glm::mat4 Projection = glm::perspective(3.14f / 4.0f, Aspect, 1.0f, 100.0f);
			const glm::mat4 Correction = glm::mat4(glm::vec4(1, 0, 0, 0), glm::vec4(0, -1, 0, 0), glm::vec4(0, 0, 1.0f / 2.0f, 1.0f / 2.0f), glm::vec4(0, 0, 0, 1));

			glm::mat4 Camera = glm::lookAt(Data.CameraPosition, Data.CameraPosition + Data.CameraForward, glm::vec3(0, 1, 0));
			glm::mat4 MV = Camera * DataToRender.Transform;
			glm::mat4 MVP = Correction * Projection * MV;

			Mesh->GetMaterial(Id)->SetMVP(MVP);
			Mesh->GetMaterial(Id)->SetMV(MV);

			Mesh->GetMaterial(Id)->Update();

			Cmd::BindVertexAndIndexBuffer(mBasePassCommandBuffer.get(), Mesh->GetVertexBuffer(Id), Mesh->GetIndexBuffer(Id));
			Cmd::UpdateDescriptorData(mBasePassCommandBuffer.get(), DescInst, Pipeline);
			Cmd::SetViewports(mBasePassCommandBuffer.get(), Pipeline);
			Cmd::DrawIndexed(mBasePassCommandBuffer.get(), Mesh->GetIndiciesSize(Id));

		}
	}

	Cmd::EndRenderPass(mBasePassCommandBuffer.get());

	mBasePassCommandBuffer->End();

	mBasePassCommandBuffer->Submit(false, { mBasePassReady.get() }, { mImageReadyToDraw[CurrentImageIndex].get() }, { PipelineStage::COLOR_ATTACHMENT, PipelineStage::FRAGMENT });



		
	Image::ChangeMultipleLayouts(	{ mColorBuffer.get(),		mNormalBuffer.get(),		mPositionBuffer.get(),		mSceneBuffer.get() },
									{ ImageLayout::SHADER_READ,	ImageLayout::SHADER_READ,	ImageLayout::SHADER_READ,	ImageLayout::COLOR_ATTACHMENT });


	// Light pass
	{
		mLightPassCommandBuffer = std::make_unique<CommandBuffer>(GraphicsQueueIndex);
		mLightPassCommandBuffer->Begin(CBUsage::ONE_TIME);

		const std::vector<VkClearValue> SceneClearColor = { { 0, 0, 0, 1 } };
		Cmd::BeginRenderPass(mLightPassCommandBuffer.get(), mLightPassFramebuffer.get(), mLightPassRenderPass.get(), SceneClearColor, Extend);

		IGraphicsPipeline* Pipeline = PipelineManager::Get().GetPipelineByKey(mDirectionalLightPassDescriporInst->GetPipelineKey());

		SamplerSettings SamplerInstSettings = {};
		SamplerInstSettings.MaxAnisotropy = 16;

		Sampler* DefaultSampler = TextureManager::Get().GetSampler(SamplerInstSettings);

		mDirectionalLightPassDescriporInst->SetImage("ColorTex", *mColorView, *DefaultSampler);
		mDirectionalLightPassDescriporInst->SetImage("NormalTex", *mNormalView, *DefaultSampler);
		mDirectionalLightPassDescriporInst->SetImage("PositionTex", *mPositionView, *DefaultSampler);
		mDirectionalLightPassDescriporInst->Update();

		auto PCFragPtr = mDirectionalLightPassDescriporInst->GetPushConstantBuffer(ShaderType::FRAGMENT);
		PCFragPtr->Set("Direction", glm::normalize(glm::vec3(-1, -1, -1)));
		PCFragPtr->Set("LightColor", glm::vec3(0,1,1));
		
		Cmd::BindGraphicsPipeline(mLightPassCommandBuffer.get(), Pipeline);

		Cmd::BindVertexBuffer(mLightPassCommandBuffer.get(), mScreenVertexBuffer.get());
		Cmd::UpdateDescriptorData(mLightPassCommandBuffer.get(), mDirectionalLightPassDescriporInst.get(), Pipeline);
		Cmd::SetViewports(mLightPassCommandBuffer.get(), Pipeline);
		Cmd::Draw(mLightPassCommandBuffer.get(), 6);

		Cmd::EndRenderPass(mLightPassCommandBuffer.get());

		mLightPassCommandBuffer->End();

		mLightPassCommandBuffer->Submit(false, { mLightPassReady.get() }, { mBasePassReady.get() }, { PipelineStage::COLOR_ATTACHMENT, PipelineStage::FRAGMENT });
	}



	Image::ChangeMultipleLayouts(	{ mSceneBuffer.get() }, 
									{ ImageLayout::SHADER_READ });



	// Screen pass
	{
		mScreenCommandBuffer = std::make_unique<CommandBuffer>(GraphicsQueueIndex);
		mScreenCommandBuffer->Begin(CBUsage::ONE_TIME);
	
		const std::vector<VkClearValue> ClearColor = { { 0, 0, 0, 1 } };
		Cmd::BeginRenderPass(mScreenCommandBuffer.get(), mFramebuffers[CurrentImageIndex].get(), mScreenRenderPass.get(), ClearColor, Extend);

		IGraphicsPipeline* Pipeline = PipelineManager::Get().GetPipelineByKey(mScreenDescriporInst->GetPipelineKey());

		SamplerSettings SamplerInstSettings = {};
		SamplerInstSettings.MaxAnisotropy = 16;

		Sampler* DefaultSampler = TextureManager::Get().GetSampler(SamplerInstSettings);
	
		mScreenDescriporInst->SetImage("SceneTex", *mSceneView, *DefaultSampler);
		mScreenDescriporInst->Update();

		Cmd::BindGraphicsPipeline(mScreenCommandBuffer.get(), Pipeline);

		Cmd::BindVertexBuffer(mScreenCommandBuffer.get(), mScreenVertexBuffer.get());
		Cmd::UpdateDescriptorData(mScreenCommandBuffer.get(), mScreenDescriporInst.get(), Pipeline);
		Cmd::SetViewports(mScreenCommandBuffer.get(), Pipeline);
		Cmd::Draw(mScreenCommandBuffer.get(), 6);

		Cmd::EndRenderPass(mScreenCommandBuffer.get());

		mScreenCommandBuffer->End();
		mScreenCommandBuffer->Submit(mFrameFence.get(), { mImageReadyToPresent[CurrentImageIndex].get() }, { mLightPassReady.get() }, { PipelineStage::COLOR_ATTACHMENT, PipelineStage::FRAGMENT });
	}

	QueuePresent(ImageIndex, mImageReadyToPresent[CurrentImageIndex].get());



}
