#pragma once
#include <stdint.h>
#include "../File/File.h"
#include "../Renderer/window.h"
#include "../Renderer/core.h"
#include "../Renderer/memory_manager.h"
#include "../Renderer/shader.h"


namespace Engine
{
	void Startup()
	{
		assert(File::Get().Startup());
		assert(Window::Get().Startup(1024, 720, "Vulkantastic"));
		assert(VulkanCore::Get().Startup(true));
		assert(MemoryManager::Get().Startup());
		assert(ShaderManager::Get().Startup());
	}

	void Shutdown()
	{
		assert(ShaderManager::Get().Shutdown());
		assert(MemoryManager::Get().Shutdown());
		assert(VulkanCore::Get().Shutdown());
		assert(Window::Get().Shutdown());
		assert(File::Get().Shutdown());
	}
}