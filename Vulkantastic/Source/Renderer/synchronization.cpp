#define NOMINMAX
#include "synchronization.h"
#include "core.h"
#include "device.h"
#include <limits>
#include "../Utilities/assert.h"

Semaphore::Semaphore()
{
	auto Device = VulkanCore::Get().GetDevice()->GetDevice();

	VkSemaphoreCreateInfo SemaphoreInfo = {};
	SemaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	Assert(vkCreateSemaphore(Device, &SemaphoreInfo, nullptr, &mSemaphore) == VK_SUCCESS);
}

Semaphore::Semaphore(Semaphore&& Rhs) noexcept
{
	*this = std::move(Rhs);
}

Semaphore& Semaphore::operator=(Semaphore&& Rhs) noexcept
{
	mSemaphore = Rhs.mSemaphore;
	Rhs.mSemaphore = nullptr;

	return *this;
}

Semaphore::~Semaphore()
{
	if (!mSemaphore) { return; }

	auto Device = VulkanCore::Get().GetDevice()->GetDevice();

	vkDestroySemaphore(Device, mSemaphore, nullptr);
}

Fence::Fence(bool Signal /*= true*/)
{
	auto Device = VulkanCore::Get().GetDevice()->GetDevice();

	VkFenceCreateInfo FenceInfo = {};
	FenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	FenceInfo.flags = Signal ? VK_FENCE_CREATE_SIGNALED_BIT : 0;

	Assert(vkCreateFence(Device, &FenceInfo, nullptr, &mFence) == VK_SUCCESS);
}

Fence::Fence(Fence&& Rhs) noexcept
{
	*this = std::move(Rhs);
}

bool Fence::Wait(uint64_t Time)
{
	auto Device = VulkanCore::Get().GetDevice()->GetDevice();

	return vkWaitForFences(Device, 1, &mFence, VK_TRUE, Time == 0 ? std::numeric_limits<uint64_t>::max() : Time) == VK_SUCCESS;
}

bool Fence::Reset()
{
	auto Device = VulkanCore::Get().GetDevice()->GetDevice();

	return vkResetFences(Device, 1, &mFence) == VK_SUCCESS;
}

Fence& Fence::operator=(Fence&& Rhs) noexcept
{
	mFence = Rhs.mFence;
	Rhs.mFence = nullptr;

	return *this;
}

Fence::~Fence()
{
	if (!mFence) { return; }

	auto Device = VulkanCore::Get().GetDevice()->GetDevice();

	vkDestroyFence(Device, mFence, nullptr);
}
