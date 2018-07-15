#pragma once
#define VK_PROTOTYPES 1
#define VK_USE_PLATFORM_WIN32_KHR 1
#include "vulkan/vulkan.h"

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

	inline VkSurfaceKHR GetSurface() const { return mSurface; }
	inline VkInstance GetInstance() const { return mInstance; }
	inline bool GetDebugMode() const { return mDebugMode; }

private:
	bool mDebugMode = false;
	VkInstance mInstance = nullptr;
	VkSurfaceKHR mSurface = nullptr;
	VkDebugReportCallbackEXT mCallback = nullptr;
	class Device* mDevice;

	VulkanCore() = default;
	bool CreateInstance();
	bool CreateWin32Surface();
	bool CreateDebugReportCallback();

};