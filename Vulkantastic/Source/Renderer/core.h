#pragma once
#define VK_PROTOTYPES 1
#define VK_USE_PLATFORM_WIN32_KHR 1
#include "vulkan/vulkan.h"

class Device;
class SwapChain;

class VulkanCore
{
public:

	static VulkanCore& Get()
	{
		static VulkanCore* instance = new VulkanCore();
		return *instance;
	}

	bool Startup(bool DebugMode = false);
	bool Shutdown();

	VkCommandPool GetGraphicsCommandPoolForCurrentThread();
	VkCommandPool GetComputeCommandPoolForCurrentThread();
	VkCommandPool GetCommandPoolByIndex(int32_t QueueIndex) const;
	int32_t ProgessImageIndex();

	inline VkSurfaceKHR GetSurface() const { return mSurface; }
	inline VkInstance GetInstance() const { return mInstance; }
	inline bool GetDebugMode() const { return mDebugMode; }
	inline Device* GetDevice() const { return mDevice; }
	inline SwapChain* GetSwapChain() const { return mSwapChain; }
	inline int32_t GetImageIndex() const { return mCurrentImageIndex; }
	VkExtent2D GetExtend() const;
	void WaitForGPU();

private:
	bool mDebugMode = false;
	VkInstance mInstance = nullptr;
	VkSurfaceKHR mSurface = nullptr;
	VkDebugReportCallbackEXT mCallback = nullptr;
	Device* mDevice;
	SwapChain* mSwapChain;
	int32_t mCurrentImageIndex = 0;

	VulkanCore() = default;
	bool CreateInstance();
	bool CreateWin32Surface();
	bool CreateDebugReportCallback();

};