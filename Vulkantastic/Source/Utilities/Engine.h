#pragma once
#include <stdint.h>
#include "../File/File.h"
#include "../Renderer/window.h"
#include "../Renderer/core.h"
#include "../Renderer/memory_manager.h"
#include "../Renderer/shader.h"
#include "../Renderer/swap_chain.h"
#include "assert.h"


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
	}

	void Shutdown()
	{
		Assert(ShaderManager::Get().Shutdown());
		Assert(MemoryManager::Get().Shutdown());
		Assert(VulkanCore::Get().Shutdown());
		Assert(Window::Get().Shutdown());
		Assert(File::Get().Shutdown());
	}
}