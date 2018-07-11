#include <stdint.h>
#include "Renderer/window.h"
#include "Renderer/core.h"
#include <assert.h>

int32_t CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{

	assert(Window::Get().Startup(1024, 720, "Vulkantastic"));
	assert(VulkanCore::Get().Startup(true));

	while (!Window::Get().ShouldWindowClose())
	{
		Window::Get().Update();



	}

	assert(VulkanCore::Get().Shutdown());
	assert(Window::Get().Shutdown());

}
