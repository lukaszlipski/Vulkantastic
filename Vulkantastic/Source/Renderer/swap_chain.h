#pragma once
#define VK_PROTOTYPES 1
#define VK_USE_PLATFORM_WIN32_KHR 1
#include "vulkan/vulkan.h"
#include <vector>

class Device;

class SwapChain
{
public:
	SwapChain(const Device* VulkanDevice);
	~SwapChain();

	inline VkSurfaceFormatKHR GetFormat() const { return mFormat; }
	inline std::vector<VkImage> GetImages() const { return mImages; }
	inline uint32_t GetImagesCount() const { return static_cast<uint32_t>(mImages.size()); }
	inline VkSwapchainKHR GetSwapChain() const { return mSwapChain; }

private:
	const Device* mVulkanDevice;
	VkSurfaceFormatKHR mFormat;
	VkPresentModeKHR mPresentMode;
	VkSwapchainKHR mSwapChain;
	std::vector<VkImage> mImages;

	void FindFormat();
	void FindPresentMode();
};