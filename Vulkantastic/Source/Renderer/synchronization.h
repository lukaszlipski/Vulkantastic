#pragma once
#include "vulkan/vulkan_core.h"

class Semaphore
{
public:
	Semaphore();
	~Semaphore();

	Semaphore(const Semaphore& Rhs) = delete;
	Semaphore& operator=(const Semaphore& Rhs) = delete;

	Semaphore(Semaphore&& Rhs) noexcept;
	Semaphore& operator=(Semaphore&& Rhs) noexcept;

	inline VkSemaphore* GetPtr() { return &mSemaphore; }
	inline VkSemaphore Get() { return mSemaphore; }

private:
	VkSemaphore mSemaphore = nullptr;

};


class Fence
{
public:
	Fence(bool Signal = true);
	~Fence();

	Fence(const Fence& Rhs) = delete;
	Fence& operator=(const Fence& Rhs) = delete;

	Fence(Fence&& Rhs) noexcept;
	Fence& operator=(Fence&& Rhs) noexcept;

	inline VkFence* GetPtr() { return &mFence; }
	inline VkFence Get() { return mFence; }

	bool Wait(uint64_t Time = 0);
	bool Reset();

private:
	VkFence mFence = nullptr;

};