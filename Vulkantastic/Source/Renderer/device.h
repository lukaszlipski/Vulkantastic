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

private:
	VkDevice mDevice = nullptr;
	QueueResult QueuesIndicies = {};
	VkQueue mGraphicsQueue = nullptr; // Assumption that graphics queue == presentation
	VkQueue mComputeQueue = nullptr;

	void GetQueues();
	bool CreateDevice(const VkPhysicalDevice& Device);
	bool FindDevice(const VkPhysicalDevice& Device);
	QueueResult FindQueueFamilies(const VkPhysicalDevice& Device);
	bool CheckDeviceExtensionSupport(const VkPhysicalDevice& Device);
	bool CheckDeviceFormatsSupport(const VkPhysicalDevice& Device);

};