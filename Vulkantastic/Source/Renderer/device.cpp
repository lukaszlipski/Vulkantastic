#include "device.h"
#include "core.h"
#include <algorithm>
#include "../Utilities/assert.h"

std::vector<const char*> DeviceExt = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};


Device::Device()
{
	auto VulkanInstance = VulkanCore::Get().GetInstance();
	
	uint32_t DeviceCount;
	vkEnumeratePhysicalDevices(VulkanInstance, &DeviceCount, nullptr);

	std::vector<VkPhysicalDevice> PhysicalDevices(DeviceCount);
	vkEnumeratePhysicalDevices(VulkanInstance, &DeviceCount, PhysicalDevices.data());

	for (auto& PhysicalDevice : PhysicalDevices)
	{
		if (FindDevice(PhysicalDevice))
		{
			CreateDevice(PhysicalDevice);
			GetCapabilities(PhysicalDevice);
			GetProperties(PhysicalDevice);
			GetQueues();
			break;
		}
	}

}

Device::~Device()
{
	vkDestroyDevice(mDevice, nullptr);
}

VkQueue Device::GetQueueByIndex(int32_t QueueIndex) const
{
	if (VulkanCore::Get().GetDevice()->GetQueuesIndicies().GraphicsIndex == QueueIndex)
	{
		return GetGraphicsQueue();
	}
	else if (VulkanCore::Get().GetDevice()->GetQueuesIndicies().ComputeIndex == QueueIndex)
	{
		return GetComputeQueue();
	}
	Assert(false); // Wrong queue index
	return nullptr;
}

void Device::GetCapabilities(const VkPhysicalDevice& Device)
{
	auto VulkanSurface = VulkanCore::Get().GetSurface();

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(Device, VulkanSurface, &mSurfaceCapabilities);

	uint32_t SurfaceFormatsCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(Device, VulkanSurface, &SurfaceFormatsCount, nullptr);
	mSurfaceFormats.clear();
	mSurfaceFormats.resize(SurfaceFormatsCount);
	vkGetPhysicalDeviceSurfaceFormatsKHR(Device, VulkanSurface, &SurfaceFormatsCount, mSurfaceFormats.data());

	uint32_t PresentModesCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(Device, VulkanSurface, &PresentModesCount, nullptr);
	mPresentModes.clear();
	mPresentModes.resize(PresentModesCount);
	vkGetPhysicalDeviceSurfacePresentModesKHR(Device, VulkanSurface, &PresentModesCount, mPresentModes.data());
}

void Device::GetProperties(const VkPhysicalDevice& Device)
{
	VkPhysicalDeviceProperties Properties;

	vkGetPhysicalDeviceProperties(Device, &Properties);

	mLimits = Properties.limits;
}

void Device::GetQueues()
{
	vkGetDeviceQueue(mDevice, mQueuesIndicies.GraphicsIndex, 0, &mGraphicsQueue);

	if (mQueuesIndicies.ComputeIndex != mQueuesIndicies.GraphicsIndex)
	{
		vkGetDeviceQueue(mDevice, mQueuesIndicies.ComputeIndex, 0, &mComputeQueue);
	}
	else
	{
		mComputeQueue = mGraphicsQueue;
	}
}

bool Device::CreateDevice(const VkPhysicalDevice& Device)
{
	VkDeviceCreateInfo DeviceCreateInfo = {};
	DeviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	
	// Queues
	std::vector<VkDeviceQueueCreateInfo> Queues;

	float Priorities = 1.0f;
	VkDeviceQueueCreateInfo GraphicsQueueCreateInfo = {};
	GraphicsQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	GraphicsQueueCreateInfo.queueCount = 1;
	GraphicsQueueCreateInfo.queueFamilyIndex = mQueuesIndicies.GraphicsIndex;
	GraphicsQueueCreateInfo.pQueuePriorities = &Priorities;

	Queues.push_back(GraphicsQueueCreateInfo);

	if (mQueuesIndicies.GraphicsIndex != mQueuesIndicies.ComputeIndex)
	{
		VkDeviceQueueCreateInfo ComputeQueueCreateInfo = {};
		ComputeQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		ComputeQueueCreateInfo.queueCount = 1;
		ComputeQueueCreateInfo.queueFamilyIndex = mQueuesIndicies.ComputeIndex;
		ComputeQueueCreateInfo.pQueuePriorities = &Priorities;

		Queues.push_back(ComputeQueueCreateInfo);
	}

	DeviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(Queues.size());
	DeviceCreateInfo.pQueueCreateInfos = Queues.data();

	// Device features
	VkPhysicalDeviceFeatures DeviceFeatures = {};
	DeviceFeatures.samplerAnisotropy = VK_TRUE;
	DeviceFeatures.fragmentStoresAndAtomics = VK_TRUE;
	DeviceFeatures.vertexPipelineStoresAndAtomics = VK_TRUE;
	DeviceFeatures.multiViewport = VK_TRUE;

	DeviceCreateInfo.pEnabledFeatures = &DeviceFeatures;

	// Extensions
	DeviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(DeviceExt.size());
	DeviceCreateInfo.ppEnabledExtensionNames = DeviceExt.data();

	// Debug layers
	if (VulkanCore::Get().GetDebugMode())
	{
		const char* DebugLayerName = "VK_LAYER_LUNARG_standard_validation";
		DeviceCreateInfo.enabledLayerCount = 1;
		DeviceCreateInfo.ppEnabledLayerNames = &DebugLayerName;	
	}

	return vkCreateDevice(Device, &DeviceCreateInfo, nullptr, &mDevice) == VK_SUCCESS;
}

bool Device::FindDevice(const VkPhysicalDevice& Device)
{

	if (!CheckDeviceExtensionSupport(Device)) { return false; }

	VkPhysicalDeviceProperties Properties;
	VkPhysicalDeviceFeatures Features;

	vkGetPhysicalDeviceProperties(Device, &Properties);
	vkGetPhysicalDeviceFeatures(Device, &Features);

	// #TODO: Check required properties and features

	QueueResult Queues = FindQueueFamilies(Device);

	if (!Queues.IsValid())
	{
		return false;
	}

	mQueuesIndicies = Queues;

	if (!CheckDeviceFormatsSupport(Device)) { return false; }

	vkGetPhysicalDeviceMemoryProperties(Device, &mMemoryProperties);

	return true;
}

QueueResult Device::FindQueueFamilies(const VkPhysicalDevice& Device)
{
	auto VulkanSurface = VulkanCore::Get().GetSurface();

	QueueResult Result = {};

	uint32_t QueuesCount;
	vkGetPhysicalDeviceQueueFamilyProperties(Device, &QueuesCount, nullptr);
	std::vector<VkQueueFamilyProperties> QueueFamiles(QueuesCount);
	vkGetPhysicalDeviceQueueFamilyProperties(Device, &QueuesCount, QueueFamiles.data());

	for (uint32_t QueueIndex = 0; QueueIndex < QueuesCount; ++QueueIndex)
	{
		VkQueueFamilyProperties& CurrentQueue = QueueFamiles[QueueIndex];

		VkBool32 IsPresentationQueueSupported;
		vkGetPhysicalDeviceSurfaceSupportKHR(Device, QueueIndex, VulkanSurface, &IsPresentationQueueSupported);

		if (CurrentQueue.queueCount <= 0) { continue; }

		if (CurrentQueue.queueFlags & VK_QUEUE_GRAPHICS_BIT && IsPresentationQueueSupported)
		{
			Result.GraphicsIndex = QueueIndex;
		}
		if (CurrentQueue.queueFlags & VK_QUEUE_COMPUTE_BIT)
		{
			Result.ComputeIndex = QueueIndex;
		}

	}

	return Result;
}

bool Device::CheckDeviceExtensionSupport(const VkPhysicalDevice& Device)
{
	uint32_t ExtCount;
	vkEnumerateDeviceExtensionProperties(Device, nullptr, &ExtCount, nullptr);

	std::vector<VkExtensionProperties> DeviceExtensions(ExtCount);
	vkEnumerateDeviceExtensionProperties(Device, nullptr, &ExtCount, DeviceExtensions.data());

	bool Result = true;
	for (auto& Ext : DeviceExt)
	{
		auto Elem = std::find_if(DeviceExtensions.begin(), DeviceExtensions.end(), [&Ext](auto& SupportedExt) {
			return strcmp(SupportedExt.extensionName, Ext) == 0;
		});
		Result &= Elem != DeviceExtensions.end();
	}
	
	return Result;
}

bool Device::CheckDeviceFormatsSupport(const VkPhysicalDevice& Device)
{
	bool Result = true;

	VkFormatProperties Props;
	vkGetPhysicalDeviceFormatProperties(Device, VK_FORMAT_D24_UNORM_S8_UINT, &Props);

	Result &= (Props.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) > 0;

	return Result;
}
