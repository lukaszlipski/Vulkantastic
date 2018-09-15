#pragma once
#define VK_PROTOTYPES 1
#define VK_USE_PLATFORM_WIN32_KHR 1
#include "vulkan/vulkan.h"
#include <vector>

struct QueueResult
{
	int32_t GraphicsIndex = -1;
	int32_t ComputeIndex = -1;

	bool IsValid() const
	{
		return GraphicsIndex >= 0 && ComputeIndex >= 0;
	}
};

class Device
{
public:
	explicit Device();
	~Device();

	VkDevice GetDevice() const { return mDevice; }
	bool IsValid() const { return mDevice && mGraphicsQueue && mComputeQueue; }

	inline std::vector<VkSurfaceFormatKHR> GetSurfaceFormats() const { return mSurfaceFormats; }
	inline std::vector<VkPresentModeKHR> GetPresentModes() const { return mPresentModes; }
	inline VkSurfaceCapabilitiesKHR GetSurfaceCapabilities() const { return mSurfaceCapabilities; }
	inline VkPhysicalDeviceMemoryProperties GetMemoryProperties() const { return mMemoryProperties; }
	inline QueueResult GetQueuesIndicies() const { return mQueuesIndicies; }
	inline VkQueue GetGraphicsQueue() const { return mGraphicsQueue; }
	inline VkQueue GetComputeQueue() const { return mComputeQueue; }
	VkQueue GetQueueByIndex(int32_t QueueIndex) const;

private:
	VkDevice mDevice = nullptr;
	QueueResult mQueuesIndicies = {};
	VkQueue mGraphicsQueue = nullptr; // Assumption that graphics queue == presentation
	VkQueue mComputeQueue = nullptr;
	std::vector<VkSurfaceFormatKHR> mSurfaceFormats;
	std::vector<VkPresentModeKHR> mPresentModes;
	VkSurfaceCapabilitiesKHR mSurfaceCapabilities;
	VkPhysicalDeviceMemoryProperties mMemoryProperties;

	void GetQueues();
	void GetCapabilities(const VkPhysicalDevice& Device);
	bool CreateDevice(const VkPhysicalDevice& Device);
	bool FindDevice(const VkPhysicalDevice& Device);
	QueueResult FindQueueFamilies(const VkPhysicalDevice& Device);
	bool CheckDeviceExtensionSupport(const VkPhysicalDevice& Device);
	bool CheckDeviceFormatsSupport(const VkPhysicalDevice& Device);

};