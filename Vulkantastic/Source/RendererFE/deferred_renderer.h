#pragma once
#include "../Renderer/synchronization.h"
#include "../Renderer/framebuffer.h"
#include "../Renderer/command_buffer.h"
#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include "static_mesh_component.h"
#include "../Renderer/pipeline_manager.h"
#include "../Renderer/uniform_buffer.h"
#include "../Renderer/shader_parameters.h"

class StaticMesh;
class DescriptorInst;
class Buffer;

struct SceneData
{
	glm::vec3 CameraPosition = { 0.0f,0.0f,0.0f };
	glm::vec3 CameraForward = { 0.0f, 0.0f, -1.0f };
	std::vector<StaticMeshComponent*> StaticMeshComponents;
};

class DeferredRenderer
{
public:
	static DeferredRenderer& Get()
	{
		static DeferredRenderer* instance = new DeferredRenderer();
		return *instance;
	}

	~DeferredRenderer();

	bool Startup();
	bool Shutdown();

	inline RenderPass* GetBasePassRenderPass() const { return mBasePassRenderPass.get(); }
	void PrepareFramebuffers();
	void PrepareSynchronizationPrimitives();

	void Render(SceneData& Data);

private:

	std::vector<upSemaphore> mImageReadyToDraw;
	std::vector<upSemaphore> mImageReadyToPresent;
	upFence mFrameFence;

	std::vector<upImageView> mImageViews;
	std::vector<upFramebuffer> mFramebuffers;

	// Base pass
	std::unique_ptr<CommandBuffer> mBasePassCommandBuffer;
	std::unique_ptr<Framebuffer> mBasePassFramebuffer;
	std::unique_ptr<RenderPass> mBasePassRenderPass;

	upImage mDepthBuffer;
	upImageView mDepthView;
	upImage mColorBuffer;
	upImageView mColorView;
	upImage mNormalBuffer;
	upImageView mNormalView;
	upImage mPositionBuffer;
	upImageView mPositionView;

	upSemaphore mBasePassReady;

	// Materials
	using UBTemplate = std::pair<uint32_t, upUniformBufferList>;
	using UBTemplates = std::vector<UBTemplate>;
	
	std::map<PipelineManager::KeyType, UBTemplates> mMaterialUniformBuffers;
	std::map<PipelineManager::KeyType, upDescriptorInstList> mDescriptorInstances;
	int32_t mMaxElementsInUB = 32;

	inline int32_t GetBufferIDByIndex(int32_t Index) { return Index / mMaxElementsInUB; }
	inline int32_t GetEntryIDByIndex(int32_t Index) { return Index % mMaxElementsInUB; }

	// Light pass
	std::unique_ptr<CommandBuffer> mLightPassCommandBuffer;
	std::unique_ptr<Framebuffer> mLightPassFramebuffer;
	std::unique_ptr<RenderPass> mLightPassRenderPass;

	upShaderParameters mDirectionalLightPassShaderParams;
	upDescriptorInst mDirectionalLightPassDescriporInst;

	upImage mSceneBuffer;
	upImageView mSceneView;

	upSemaphore mLightPassReady;

	// Screen
	std::unique_ptr<CommandBuffer> mScreenCommandBuffer;
	std::unique_ptr<RenderPass> mScreenRenderPass;

	upShaderParameters mScreenShaderParams;
	upDescriptorInst mScreenDescriporInst;

	std::unique_ptr<Buffer> mScreenVertexBuffer;


};
