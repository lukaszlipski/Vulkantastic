#include "core.h"
#include "window.h"
#include "device.h"
#include "swap_chain.h"
#include "../Utilities/assert.h"

std::vector<const char*> InstanceExt = {
	VK_KHR_SURFACE_EXTENSION_NAME,
	VK_KHR_WIN32_SURFACE_EXTENSION_NAME
};

VkBool32 VulcanDebugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location, int32_t messageCode, const char* pLayerPrefix, const char* pMessage, void* pUserData);

bool VulkanCore::Startup(bool DebugMode)
{
	mDebugMode = DebugMode;

	if (!CreateInstance()) { return false; }
	if (!CreateWin32Surface()) { return false; }
	if (mDebugMode && !CreateDebugReportCallback()) { return false; }

	mDevice = new Device();
	if (!mDevice->IsValid()) { return false; }

	mSwapChain = new SwapChain(mDevice);

	return true;
}

bool VulkanCore::Shutdown()
{
	vkDeviceWaitIdle(mDevice->GetDevice());

	vkDestroyCommandPool(mDevice->GetDevice(), GetGraphicsCommandPoolForCurrentThread(), nullptr);
	vkDestroyCommandPool(mDevice->GetDevice(), GetComputeCommandPoolForCurrentThread(), nullptr);

	delete mSwapChain;
	delete mDevice;

	if (mDebugMode)
	{
		auto DestroyDebugReportCallbackEXT = reinterpret_cast<PFN_vkDestroyDebugReportCallbackEXT>(vkGetInstanceProcAddr(mInstance, "vkDestroyDebugReportCallbackEXT"));
		if (DestroyDebugReportCallbackEXT)
		{
			DestroyDebugReportCallbackEXT(mInstance, mCallback, nullptr);
		}
	}

	vkDestroySurfaceKHR(mInstance, mSurface, nullptr);
	vkDestroyInstance(mInstance, nullptr);

	return true;
}

VkCommandPool VulkanCore::GetGraphicsCommandPoolForCurrentThread()
{
	thread_local VkCommandPool ThreadLocalCommandPool;
	if (!ThreadLocalCommandPool)
	{
		auto Device = VulkanCore::Get().GetDevice()->GetDevice();

		VkCommandPoolCreateInfo CommandPoolInfo = {};
		CommandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		CommandPoolInfo.queueFamilyIndex = VulkanCore::Get().GetDevice()->GetQueuesIndicies().GraphicsIndex;
		CommandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

		Assert(vkCreateCommandPool(Device, &CommandPoolInfo, nullptr, &ThreadLocalCommandPool) == VK_SUCCESS);

	}
	return ThreadLocalCommandPool;
}

VkCommandPool VulkanCore::GetComputeCommandPoolForCurrentThread()
{
	thread_local VkCommandPool ThreadLocalCommandPool;
	if (!ThreadLocalCommandPool)
	{
		auto Device = VulkanCore::Get().GetDevice()->GetDevice();

		VkCommandPoolCreateInfo CommandPoolInfo = {};
		CommandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		CommandPoolInfo.queueFamilyIndex = VulkanCore::Get().GetDevice()->GetQueuesIndicies().ComputeIndex;
		CommandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

		Assert(vkCreateCommandPool(Device, &CommandPoolInfo, nullptr, &ThreadLocalCommandPool) == VK_SUCCESS);

	}
	return ThreadLocalCommandPool;
}

VkCommandPool VulkanCore::GetCommandPoolByIndex(int32_t QueueIndex) const
{
	if (Get().GetDevice()->GetQueuesIndicies().GraphicsIndex == QueueIndex)
	{
		return Get().GetGraphicsCommandPoolForCurrentThread();
	}
	else if (Get().GetDevice()->GetQueuesIndicies().ComputeIndex == QueueIndex)
	{
		return Get().GetComputeCommandPoolForCurrentThread();
	}
	Assert(false); // Wrong queue index
	return nullptr;
}

int32_t VulkanCore::ProgessImageIndex()
{
	auto ImageCount = mSwapChain->GetImagesCount();
	mCurrentImageIndex = (mCurrentImageIndex + 1) % ImageCount;

	return mCurrentImageIndex;
}

VkExtent2D VulkanCore::GetExtend() const
{
	return GetDevice()->GetSurfaceCapabilities().currentExtent;
}

void VulkanCore::WaitForGPU()
{
	auto Device = VulkanCore::Get().GetDevice()->GetDevice();
	vkDeviceWaitIdle(Device);
}

bool VulkanCore::CreateInstance()
{
	VkApplicationInfo AppInfo = {};
	AppInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	AppInfo.apiVersion = VK_API_VERSION_1_1;
	AppInfo.pApplicationName = Window::Get().GetTitle().c_str();
	AppInfo.pEngineName = "Vulkantastic";
	AppInfo.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
	AppInfo.engineVersion = VK_MAKE_VERSION(0, 1, 0);

	VkInstanceCreateInfo InstInfo = {};
	InstInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	InstInfo.pApplicationInfo = &AppInfo;

	if (GetDebugMode())
	{
		const char* DebugLayerName = "VK_LAYER_LUNARG_standard_validation";
		InstInfo.enabledLayerCount = 1;
		InstInfo.ppEnabledLayerNames = &DebugLayerName;
		InstanceExt.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
	}

	InstInfo.enabledExtensionCount = static_cast<uint32_t>(InstanceExt.size());
	InstInfo.ppEnabledExtensionNames = InstanceExt.data();

	if (vkCreateInstance(&InstInfo, nullptr, &mInstance) != VK_SUCCESS)
	{
		return false;
	}

	return true;
}

bool VulkanCore::CreateWin32Surface()
{
	VkWin32SurfaceCreateInfoKHR SurfaceInfo = {};
	SurfaceInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	SurfaceInfo.hwnd = Window::Get().GetWindowHandleWin32();
	SurfaceInfo.hinstance = GetModuleHandle(nullptr);

	return vkCreateWin32SurfaceKHR(mInstance, &SurfaceInfo, nullptr, &mSurface) == VK_SUCCESS;
}

bool VulkanCore::CreateDebugReportCallback()
{
	VkDebugReportCallbackCreateInfoEXT DebugCallbackInfo = {};
	DebugCallbackInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
	DebugCallbackInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
	DebugCallbackInfo.pfnCallback = &VulcanDebugCallback;

	auto CreateDebugReportCallbackEXT = reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>(vkGetInstanceProcAddr(mInstance, "vkCreateDebugReportCallbackEXT"));

	return CreateDebugReportCallbackEXT && CreateDebugReportCallbackEXT(mInstance, &DebugCallbackInfo, nullptr, &mCallback) == VK_SUCCESS;
}

VkBool32 VulcanDebugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location, int32_t messageCode, const char* pLayerPrefix, const char* pMessage, void* pUserData)
{
	OutputDebugString(pMessage);
	OutputDebugString("\n");

	return VK_FALSE;
}
