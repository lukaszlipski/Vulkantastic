#pragma once
#include <memory>
#include <vector>
#include "../Renderer/synchronization.h"
#include "../Renderer/framebuffer.h"
#include "../Renderer/command_buffer.h"
#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include "static_mesh_component.h"

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

	// Light pass
	std::unique_ptr<CommandBuffer> mLightPassCommandBuffer;
	std::unique_ptr<Framebuffer> mLightPassFramebuffer;
	std::unique_ptr<RenderPass> mLightPassRenderPass;

	std::unique_ptr<DescriptorInst> mDirectionalLightPassDescriporInst;

	upImage mSceneBuffer;
	upImageView mSceneView;

	upSemaphore mLightPassReady;

	// Screen
	std::unique_ptr<CommandBuffer> mScreenCommandBuffer;
	std::unique_ptr<RenderPass> mScreenRenderPass;
	std::unique_ptr<DescriptorInst> mScreenDescriporInst;
	std::unique_ptr<Buffer> mScreenVertexBuffer;


};
