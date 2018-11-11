#pragma once
#include <stdint.h>
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
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include "assert.h"
#include "../RendererFE/static_mesh.h"


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
	}

	void Shutdown()
	{
		Assert(StaticMeshManager::Get().Shutdown());
		Assert(ShaderManager::Get().Shutdown());
		Assert(MemoryManager::Get().Shutdown());
		Assert(VulkanCore::Get().Shutdown());
		Assert(Window::Get().Shutdown());
		Assert(File::Get().Shutdown());
	}
}