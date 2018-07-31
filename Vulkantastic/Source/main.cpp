#include <stdint.h>
#include "Renderer/window.h"
#include "Renderer/core.h"
#include <assert.h>
#include "File/File.h"
#include "Renderer/shader.h"

int32_t CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{

	assert(File::Get().Startup());
	assert(Window::Get().Startup(1024, 720, "Vulkantastic"));
	assert(VulkanCore::Get().Startup(true));
	assert(ShaderManager::Get().Startup());

	Shader* FragmentShader = ShaderManager::Get().Find("first.frag");

	while (!Window::Get().ShouldWindowClose())
	{
		Window::Get().Update();

		

	}

	assert(ShaderManager::Get().Shutdown());
	assert(VulkanCore::Get().Shutdown());
	assert(Window::Get().Shutdown());
	assert(File::Get().Shutdown());

}
