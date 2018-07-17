#pragma once
#define VK_PROTOTYPES 1
#define VK_USE_PLATFORM_WIN32_KHR 1
#include "vulkan/vulkan.h"

class Device;

class SwapChain
{
public:
	SwapChain(const Device* VulkanDevice);
	~SwapChain();

private:
	const Device* mVulkanDevice;
	VkSurfaceFormatKHR mFormat;
	VkPresentModeKHR mPresentMode;
	VkSwapchainKHR mSwapChain;

	void FindFormat();
	void FindPresentMode();
};