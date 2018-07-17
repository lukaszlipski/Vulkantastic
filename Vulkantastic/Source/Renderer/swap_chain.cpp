#include "swap_chain.h"
#include "device.h"
#include <assert.h>
#include <algorithm>
#include "core.h"

SwapChain::SwapChain(const Device* VulkanDevice)
	: mVulkanDevice(VulkanDevice)
{
	FindFormat();
	FindPresentMode();

	auto VulkanSurface = VulkanCore::Get().GetSurface();
	auto SurfaceCapabilities = mVulkanDevice->GetSurfaceCapabilities();
	assert(2 >= SurfaceCapabilities.minImageCount);

	VkSwapchainCreateInfoKHR SwapchainInfo = {};
	SwapchainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	SwapchainInfo.clipped = VK_TRUE;
	SwapchainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	SwapchainInfo.imageArrayLayers = 1;
	SwapchainInfo.imageColorSpace = mFormat.colorSpace;
	SwapchainInfo.imageFormat = mFormat.format;
	SwapchainInfo.imageExtent = SurfaceCapabilities.currentExtent;
	SwapchainInfo.minImageCount = min(SurfaceCapabilities.minImageCount,3);
	SwapchainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	SwapchainInfo.surface = VulkanSurface;
	SwapchainInfo.presentMode = mPresentMode;
	SwapchainInfo.oldSwapchain = VK_NULL_HANDLE;
	SwapchainInfo.preTransform = SurfaceCapabilities.currentTransform;
	SwapchainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	SwapchainInfo.queueFamilyIndexCount = 0;
	SwapchainInfo.pQueueFamilyIndices = nullptr;

	assert(vkCreateSwapchainKHR(mVulkanDevice->GetDevice(), &SwapchainInfo, nullptr, &mSwapChain) == VK_SUCCESS);

}

SwapChain::~SwapChain()
{
	vkDestroySwapchainKHR(mVulkanDevice->GetDevice(), mSwapChain, 0);
}

void SwapChain::FindFormat()
{
	auto Formats = mVulkanDevice->GetSurfaceFormats();

	auto RequestedFormat = std::find_if(Formats.begin(), Formats.end(), [](auto& CurrentSF) {
		const bool SupportedFormat = CurrentSF.format == VK_FORMAT_B8G8R8A8_UNORM || CurrentSF.format == VK_FORMAT_R8G8B8A8_UNORM;
		const bool SupportedColorSpace = CurrentSF.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;

		return SupportedFormat && SupportedColorSpace;
	});

	assert(RequestedFormat != Formats.end());

	mFormat = *RequestedFormat;
}

void SwapChain::FindPresentMode()
{
	auto PresentModes = mVulkanDevice->GetPresentModes();

	auto RequestedPresentMode = std::find(PresentModes.begin(), PresentModes.end(), VK_PRESENT_MODE_MAILBOX_KHR);

	if (RequestedPresentMode == PresentModes.end())
	{
		RequestedPresentMode = std::find(PresentModes.begin(), PresentModes.end(), VK_PRESENT_MODE_FIFO_KHR);
	}

	assert(RequestedPresentMode != PresentModes.end());

	mPresentMode = *RequestedPresentMode;
}
