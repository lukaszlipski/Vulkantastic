#include "Utilities/Engine.h"
#include <array>

int32_t CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	Engine::Startup();

	auto Device = VulkanCore::Get().GetDevice()->GetDevice();
	VkExtent2D Extend = VulkanCore::Get().GetExtend();

	// Load mesh
	auto* Mesh = StaticMeshManager::Get().Find("test2");

	SceneData DataToRender;
	DataToRender.CameraPosition = glm::vec3(3, 3, 3);
	DataToRender.CameraForward = glm::normalize(glm::vec3(-1, -1, -1));

	DataToRender.StaticMeshes.push_back(Mesh);

	while (!Window::Get().ShouldWindowClose())
	{
		Window::Get().Update();

		DeferredRenderer::Get().Render(DataToRender);

		Mesh->GetMaterial(0)->SetCustomColor(glm::vec3(1,0,0));

		VulkanCore::Get().ProgessImageIndex();
	}

	Engine::Shutdown();
}
