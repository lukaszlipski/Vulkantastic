#include "Utilities/Engine.h"

int32_t CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	Engine::Startup();

	Shader* FragmentShader = ShaderManager::Get().Find("first.frag");

	while (!Window::Get().ShouldWindowClose())
	{
		Window::Get().Update();

		

	}

	Engine::Shutdown();
}
