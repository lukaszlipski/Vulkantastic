#pragma once
#include <stdint.h>
#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include "assert.h"
#include "../File/File.h"
#include "../Renderer/window.h"
#include "../Renderer/core.h"
#include "../Renderer/memory_manager.h"
#include "../Renderer/shader.h"
#include "../Renderer/swap_chain.h"
#include "../Renderer/vertex_definitions.h"
#include "../Renderer/buffer.h"
#include "../Renderer/image.h"
#include "../Renderer/command_buffer.h"
#include "../Renderer/image_view.h"
#include "../Renderer/sampler.h"
#include "../Renderer/pipeline.h"
#include "../Renderer/framebuffer.h"
#include "../Renderer/uniform_buffer.h"
#include "../Renderer/push_constant_buffer.h"
#include "../Renderer/synchronization.h"
#include "../RendererFE/static_mesh.h"
#include "../Renderer/pipeline_manager.h"
#include "../RendererFE/deferred_renderer.h"
#include "../RendererFE/surface_material.h"
#include "../RendererFE/texture_manager.h"
#include "../RendererFE/static_mesh_component.h"


namespace Engine
{
	void Startup()
	{
		Assert(File::Get().Startup());
		Assert(Window::Get().Startup(1024, 720, "Vulkantastic"));
#ifdef _DEBUG
		Assert(VulkanCore::Get().Startup(true));
#else
		Assert(VulkanCore::Get().Startup(false));
#endif
		Assert(MemoryManager::Get().Startup());
		Assert(ShaderManager::Get().Startup());
		Assert(StaticMeshManager::Get().Startup());
		Assert(PipelineManager::Get().Startup());
		Assert(TextureManager::Get().Startup());
		Assert(DeferredRenderer::Get().Startup());
	}

	void Shutdown()
	{
		VulkanCore::Get().WaitForGPU();

		Assert(DeferredRenderer::Get().Shutdown());
		Assert(TextureManager::Get().Shutdown());
		Assert(PipelineManager::Get().Shutdown());
		Assert(StaticMeshManager::Get().Shutdown());
		Assert(ShaderManager::Get().Shutdown());
		Assert(MemoryManager::Get().Shutdown());
		Assert(VulkanCore::Get().Shutdown());
		Assert(Window::Get().Shutdown());
		Assert(File::Get().Shutdown());
	}
}