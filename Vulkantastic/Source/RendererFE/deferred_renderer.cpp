#define NOMINMAX
#include <limits>
#include "deferred_renderer.h"
#include "static_mesh.h"
#include "../Renderer/render_pass.h"
#include "../Renderer/core.h"
#include "../Renderer/swap_chain.h"
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
		std::vector<uint32_t> Queues = { GraphicsQueueIndex };

		Shader* VertexShader = ShaderManager::Get().Find("LightPass.vert");
		Shader* FragmentShader = ShaderManager::Get().Find("DirectionalLightPass.frag");

		Assert(VertexShader && FragmentShader);

		PipelineShaders Shaders{ VertexShader, FragmentShader };

		mDirectionalLightPassShaderParams = PipelineManager::Get().GetShaderParametersInstance<VertexDefinition::SimpleScreen>(*mLightPassRenderPass, Shaders);

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
		
		mScreenShaderParams = PipelineManager::Get().GetShaderParametersInstance<VertexDefinition::SimpleScreen>(*mScreenRenderPass, Shaders);
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
	mMaterialUniformBuffers.clear();
	mDescriptorInstances.clear();

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
		StaticMeshHandle* MeshHandle;
		glm::mat4 Transform;
		int32_t Id = -1;
	};

	using RenderableDataList = std::vector<RenderableData>;

	std::map<PipelineManager::KeyType, RenderableDataList> PartitionedRendererData;

	// Sort meshes based on materials that they use
	for (StaticMeshComponent* Mesh : Data.StaticMeshComponents)
	{
		auto CurrentMeshHandle = Mesh->GetMeshHandle();

		if(!CurrentMeshHandle) { continue;}

		for (int32_t i = 0; i < CurrentMeshHandle->GetMaterialsCount(); ++i)
		{
			const StaticSurfaceMaterial* Material = CurrentMeshHandle->GetMaterial(i);
			const PipelineManager::KeyType Key = Material->GetShaderParameters()->GetPipelineKey();
			
			RenderableData NewData = {};
			NewData.Id = i;
			NewData.MeshHandle = CurrentMeshHandle;
			NewData.Transform = Mesh->GetTransform();

			PartitionedRendererData[Key].emplace_back(std::move(NewData));
		}
	}

	// Update mvp
	for (auto& PartitionedData : PartitionedRendererData)
	{
		const PipelineManager::KeyType PipelineKey = PartitionedData.first;
		std::vector<RenderableData>& RenderableDataList = PartitionedData.second;

		for (RenderableData& DataToRender : RenderableDataList)
		{
			StaticMeshHandle* const MeshHandle = DataToRender.MeshHandle;
			const int32_t Id = DataToRender.Id;
			ShaderParameters* Params = MeshHandle->GetMaterial(Id)->GetShaderParameters();

			const float Aspect = Extend.width / float(Extend.height);
			const glm::mat4 Projection = glm::perspective(3.14f / 4.0f, Aspect, 1.0f, 100.0f);
			const glm::mat4 Correction = glm::mat4(glm::vec4(1, 0, 0, 0), glm::vec4(0, -1, 0, 0), glm::vec4(0, 0, 1.0f / 2.0f, 1.0f / 2.0f), glm::vec4(0, 0, 0, 1));

			glm::mat4 Camera = glm::lookAt(Data.CameraPosition, Data.CameraPosition + Data.CameraForward, glm::vec3(0, 1, 0));
			glm::mat4 MV = Camera * DataToRender.Transform;
			glm::mat4 MVP = Correction * Projection * MV;

			MeshHandle->GetMaterial(Id)->SetMVP(MVP);
			MeshHandle->GetMaterial(Id)->SetMV(MV);
		}
	}


	// Create uniform buffers that will hold renderable's data

	const uint32_t UniformSetIndex = 1;

	mMaterialUniformBuffers.clear(); // #TODO: Maybe there should be considered keeping uniform buffers for pipelines that are going to be used
	mDescriptorInstances.clear(); // #TODO: Same as with uniform buffers

	for (const auto& RendererData : PartitionedRendererData)
	{
		const PipelineManager::KeyType& Key = RendererData.first;
		const RenderableDataList& DataList = RendererData.second;

		UBTemplates& ubList = mMaterialUniformBuffers[Key];

		const int32_t Elements = static_cast<int32_t>(DataList.size());
		const int32_t NeededNum = (Elements / mMaxElementsInUB) + 1;

		DescriptorManager* DescManager = PipelineManager::Get().GetPipelineByKey(Key)->GetDescriptorManager();

		auto Uniforms = DescManager->GetUniforms(UniformSetIndex);

		// Create dynamic uniform buffers that are needed by materials
		for (const Uniform& Template : Uniforms)
		{
			if (Template.Format == VariableType::STRUCTURE) // Uniform buffer
			{
				uint32_t QueueIndex = VulkanCore::Get().GetDevice()->GetQueuesIndicies().GraphicsIndex;
				std::vector<uint32_t> QueueIndicies = { QueueIndex };

				upUniformBufferList NewList(NeededNum);
				for (auto& Elem : NewList)
				{
					Elem = std::make_unique<UniformBuffer>(Template, QueueIndicies, mMaxElementsInUB);
				}

				ubList.emplace_back(Template.Binding, std::move(NewList));
			}
		}

		// Update uniform buffers
		for (int32_t i = 0; i < DataList.size(); ++i)
		{
			const RenderableData& Renderable = DataList[i];
			ShaderParameters* const Params = Renderable.MeshHandle->GetMaterial(Renderable.Id)->GetShaderParameters();

			const int32_t BufferID = GetBufferIDByIndex(i);
			const int32_t EntryID = GetEntryIDByIndex(i);

			for (auto& Template : ubList)
			{
				uint32_t Binding = Template.first;
				auto& Buffers = Template.second;

				UniformRawData* UB = Params->GetUniformBufferByBinding(Binding);

				Assert(UB);

				Buffers[BufferID]->Set(UB, i);

			}	
		}

		// Upload data to uniform buffers
		for (auto& UniformBuffersForOneBinding : ubList)
		{
			auto& Buffers = UniformBuffersForOneBinding.second;
			
			for (auto& Buffer : Buffers)
			{
				Buffer->Update();
			}
		}

		// Create descriptor instances
		upDescriptorInstList& dsList = mDescriptorInstances[Key];

		for (int32_t i = 0; i < NeededNum; ++i)
		{
			upDescriptorInst NewDS = DescManager->GetDescriptorInstance(UniformSetIndex);

			dsList.push_back(std::move(NewDS));
		}

		// Update descriptor instances
		for (int32_t i = 0; i < dsList.size(); ++i)
		{
			upDescriptorInst& DS = dsList[i];

			for (const UBTemplate& Template : ubList)
			{
				DS->SetBuffer(Template.first, Template.second[i].get());
			}
		}

		for (auto& DS : dsList)
		{
			DS->Update();
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

		upDescriptorInstList& DSList = mDescriptorInstances[PipelineKey];
		UBTemplates& UBList = mMaterialUniformBuffers[PipelineKey];

		Cmd::BindGraphicsPipeline(mBasePassCommandBuffer.get(), Pipeline);

		for (int32_t i = 0; i < RenderableDataList.size(); ++i)
		{

			RenderableData& DataToRender = RenderableDataList[i];
			StaticMeshHandle* const MeshHandle = DataToRender.MeshHandle;
			const StaticMesh* const Mesh = MeshHandle->GetStaticMesh();
			const int32_t Id = DataToRender.Id;
			ShaderParameters* Params = MeshHandle->GetMaterial(Id)->GetShaderParameters();

			const int32_t BufferID = GetBufferIDByIndex(i);
			const int32_t EntryID = GetEntryIDByIndex(i);

			// Calculate a dynamic offset for each dynamic uniform buffer in a descriptor set
			std::vector<uint32_t> DynamicOffsets;
			DynamicOffsets.reserve(UBList.size());
			for (int32_t j = 0; j < UBList.size(); ++j)
			{
				const auto& FirstBuffer = UBList[j].second[0];
				const int32_t dynamicOffset = i * FirstBuffer->GetAlignmentSize();

				DynamicOffsets.push_back(dynamicOffset);
			}

			upDescriptorInst& CurrentDS = DSList[BufferID];

			Cmd::UpdateDescriptorData(mBasePassCommandBuffer.get(), Params, CurrentDS.get(), Pipeline, DynamicOffsets);

			Cmd::BindVertexAndIndexBuffer(mBasePassCommandBuffer.get(), Mesh->GetVertexBuffer(Id), Mesh->GetIndexBuffer(Id));
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

		IGraphicsPipeline* Pipeline = PipelineManager::Get().GetPipelineByKey(mDirectionalLightPassShaderParams->GetPipelineKey());

		SamplerSettings SamplerInstSettings = {};
		SamplerInstSettings.MaxAnisotropy = 16;

		Sampler* DefaultSampler = TextureManager::Get().GetSampler(SamplerInstSettings);

		mDirectionalLightPassDescriporInst->SetImage(0, mColorView.get(), DefaultSampler);
		mDirectionalLightPassDescriporInst->SetImage(1, mNormalView.get(), DefaultSampler);
		mDirectionalLightPassDescriporInst->SetImage(2, mPositionView.get(), DefaultSampler);
		mDirectionalLightPassDescriporInst->Update();

		auto PCFragPtr = mDirectionalLightPassShaderParams->GetPushConstantBuffer(ShaderType::FRAGMENT);
		PCFragPtr->Set("Direction", glm::normalize(glm::vec3(-1, -1, -1)));
		PCFragPtr->Set("LightColor", glm::vec3(0,1,1));
		
		Cmd::UpdateDescriptorData(mLightPassCommandBuffer.get(), mDirectionalLightPassShaderParams.get(), mDirectionalLightPassDescriporInst.get(), Pipeline);

		Cmd::BindGraphicsPipeline(mLightPassCommandBuffer.get(), Pipeline);
		Cmd::BindVertexBuffer(mLightPassCommandBuffer.get(), mScreenVertexBuffer.get());
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

		IGraphicsPipeline* Pipeline = PipelineManager::Get().GetPipelineByKey(mScreenShaderParams->GetPipelineKey());

		SamplerSettings SamplerInstSettings = {};
		SamplerInstSettings.MaxAnisotropy = 16;

		Sampler* DefaultSampler = TextureManager::Get().GetSampler(SamplerInstSettings);
	
		mScreenDescriporInst->SetImage(0, mSceneView.get(), DefaultSampler);
		mScreenDescriporInst->Update();

		Cmd::BindGraphicsPipeline(mScreenCommandBuffer.get(), Pipeline);

		Cmd::BindVertexBuffer(mScreenCommandBuffer.get(), mScreenVertexBuffer.get());
		
		Cmd::UpdateDescriptorData(mScreenCommandBuffer.get(), mScreenShaderParams.get(), mScreenDescriporInst.get(), Pipeline);

		Cmd::SetViewports(mScreenCommandBuffer.get(), Pipeline);
		Cmd::Draw(mScreenCommandBuffer.get(), 6);

		Cmd::EndRenderPass(mScreenCommandBuffer.get());

		mScreenCommandBuffer->End();
		mScreenCommandBuffer->Submit(mFrameFence.get(), { mImageReadyToPresent[CurrentImageIndex].get() }, { mLightPassReady.get() }, { PipelineStage::COLOR_ATTACHMENT, PipelineStage::FRAGMENT });
	}

	QueuePresent(ImageIndex, mImageReadyToPresent[CurrentImageIndex].get());



}
