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
	std::unique_ptr<CommandBuffer> mCommandBuffer;

	std::vector<Semaphore> mImageReadyToDraw;
	std::vector<Semaphore> mImageReadyToPresent;
	Fence mFrameFence;

	std::vector<std::unique_ptr<ImageView>> mImageViews;
	std::vector<std::unique_ptr<Framebuffer>> mFramebuffers;

	// Base pass
	std::unique_ptr<RenderPass> mBasePassRenderPass;
	std::unique_ptr<Image> mDepthBuffer;
	std::unique_ptr<ImageView> mDepthView;

};
