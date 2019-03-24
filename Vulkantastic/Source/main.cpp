#include "Utilities/Engine.h"
#include <array>

int32_t CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	Engine::Startup();

	auto Device = VulkanCore::Get().GetDevice()->GetDevice();
	VkExtent2D Extend = VulkanCore::Get().GetExtend();

	// Load mesh
	auto* Mesh = StaticMeshManager::Get().Find("test2");

	StaticMeshComponent MeshComp("test2");
	MeshComp.SetPosition({ 1,0,0 });
	MeshComp.SetRotation({1,0,0}, 45.0f);

	StaticMeshComponent MeshComp2(MeshComp);
	MeshComp2.SetPosition({ -1,0,0 });
	MeshComp2.SetRotation({ 1,0,0 }, -45.0f);

	SceneData DataToRender;
	DataToRender.CameraPosition = glm::vec3(3, 3, 3);
	DataToRender.CameraForward = glm::normalize(glm::vec3(-1, -1, -1));

	DataToRender.StaticMeshComponents.push_back(&MeshComp);
	DataToRender.StaticMeshComponents.push_back(&MeshComp2);

	while (!Window::Get().ShouldWindowClose())
	{
		Window::Get().Update();

		DeferredRenderer::Get().Render(DataToRender);

		Mesh->GetMaterial(0)->SetCustomColor(glm::vec3(1,1,1));

		VulkanCore::Get().ProgessImageIndex();
	}

	Engine::Shutdown();
}
