#include "Utilities/Engine.h"
#include <array>

int32_t CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	Engine::Startup();

	{
		auto Device = VulkanCore::Get().GetDevice()->GetDevice();
		VkExtent2D Extend = VulkanCore::Get().GetExtend();

		// Load mesh
		auto* Mesh = StaticMeshManager::Get().Find("test2");

		// Create vertex buffer
		uint32_t GraphicsQueueIndex = VulkanCore::Get().GetDevice()->GetQueuesIndicies().GraphicsIndex;

		// Create image buffer
		int32_t Width, Height, Comp;
		stbi_uc* Pixels = stbi_load("Textures/test.jpg", &Width, &Height, &Comp, STBI_rgb_alpha);
		const int32_t ImageSize = Width * Height * STBI_rgb_alpha;

		ImageSettings Settings = {};
		Settings.Depth = 1;
		Settings.Height = Height;
		Settings.Width = Width;
		Settings.Format = ImageFormat::R8G8B8A8_SRGB;
		Settings.Type = ImageType::TWODIM;
		Settings.Mipmaps = true;

		Image ImageBuffer({ GraphicsQueueIndex }, ImageUsage::SAMPLED | ImageUsage::TRANSFER_DST | ImageUsage::TRANSFER_SRC, true, Settings, Pixels);
		ImageBuffer.ChangeLayout(ImageLayout::SHADER_READ);

		// Create image view
		ImageViewSettings ViewSettings = {};
		ViewSettings.MipMapLevelCount = ImageBuffer.GetMipMapsCount();
		ViewSettings.Format = ImageFormat::R8G8B8A8_SRGB;

		ImageView View(&ImageBuffer, ViewSettings);

		// Create depth buffer
		ImageSettings DepthSettings = {};
		DepthSettings.Depth = 1;
		DepthSettings.Height = Extend.height;
		DepthSettings.Width = Extend.width;
		DepthSettings.Format = ImageFormat::D24S8;
		DepthSettings.Type = ImageType::TWODIM;
		DepthSettings.Mipmaps = false;

		Image DepthBuffer({ GraphicsQueueIndex }, ImageUsage::DEPTH_ATTACHMENT, true, DepthSettings);
		DepthBuffer.ChangeLayout(ImageLayout::DEPTH_STENCIL_ATTACHMENT);

		// Create depth view
		ImageViewSettings DepthViewSettings = {};
		DepthViewSettings.Format = ImageFormat::D24S8;

		ImageView DepthView(&DepthBuffer, DepthViewSettings);

		// Create sampler
		SamplerSettings SamplerInstSettings = {};
		SamplerInstSettings.MaxAnisotropy = 16;

		Sampler SamplerInst(SamplerInstSettings);

		// Create material
		auto Material = std::make_unique<StaticSurfaceMaterial>("StaticBasePass.vert", "StaticBasePass.frag");

		Material->SetCustomColor(glm::vec3(0, 1, 0)).SetAlbedoTexture(View, SamplerInst);

		Mesh->SetMaterial(0, std::move(Material));

		SceneData DataToRender;
		DataToRender.CameraPosition = glm::vec3(3, 3, 3);
		DataToRender.CameraForward = glm::normalize(glm::vec3(-1, -1, -1));

		DataToRender.StaticMeshes.push_back(Mesh);

		while (!Window::Get().ShouldWindowClose())
		{
			Window::Get().Update();

			DeferredRenderer::Get().Render(DataToRender);

			VulkanCore::Get().ProgessImageIndex();
		}

		vkDeviceWaitIdle(Device);
	}


	Engine::Shutdown();
}
