#define NOMINMAX
#include "command_buffer.h"
#include "../Utilities/assert.h"
#include "core.h"
#include "device.h"
#include <limits>
#include "synchronization.h"
#include <algorithm>

CommandBuffer::CommandBuffer(int32_t QueueIndex)
	: mQueueIndex(QueueIndex)
{
	const auto Device = VulkanCore::Get().GetDevice()->GetDevice();

	VkCommandBufferAllocateInfo CommandBufferAllocateInfo = {};
	CommandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	CommandBufferAllocateInfo.commandBufferCount = 1;
	CommandBufferAllocateInfo.commandPool = VulkanCore::Get().GetCommandPoolByIndex(mQueueIndex);
	CommandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

	Assert(vkAllocateCommandBuffers(Device, &CommandBufferAllocateInfo, &mCommandBuffer) == VK_SUCCESS);
}

CommandBuffer::~CommandBuffer()
{
	const auto Device = VulkanCore::Get().GetDevice()->GetDevice();
	vkFreeCommandBuffers(Device, VulkanCore::Get().GetCommandPoolByIndex(mQueueIndex), 1, &mCommandBuffer);
}

void CommandBuffer::Begin(CBUsage Usage)
{
	VkCommandBufferBeginInfo CommandBeginInfo = {};
	CommandBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	CommandBeginInfo.flags = static_cast<VkCommandBufferUsageFlags>(Usage);
	
	Assert(vkBeginCommandBuffer(mCommandBuffer, &CommandBeginInfo) == VK_SUCCESS);
}

void CommandBuffer::End()
{
	Assert(vkEndCommandBuffer(mCommandBuffer) == VK_SUCCESS);

}

void CommandBuffer::Submit(bool Wait, std::vector<Semaphore*> Signal, std::vector<Semaphore*> WaitFor, std::vector<PipelineStage> WaitStage)
{

	if (Wait)
	{
		Fence WaitFence(false);
		Submit(&WaitFence, Signal, WaitFor, WaitStage);
		WaitFence.Wait();
	}
	else
	{
		Submit(nullptr, Signal, WaitFor, WaitStage);
	}

}

void CommandBuffer::Submit(Fence* CustomFence, std::vector<Semaphore*> Signal, std::vector<Semaphore*> WaitFor, std::vector<PipelineStage> WaitStage)
{
	const auto Queue = VulkanCore::Get().GetDevice()->GetQueueByIndex(mQueueIndex);

	std::vector<VkSemaphore> RawSignalSemaphores(Signal.size());
	std::vector<VkSemaphore> RawWaitForSemaphores(WaitFor.size());

	std::transform(Signal.begin(), Signal.end(), RawSignalSemaphores.begin(), [](auto* ElemA) {
		return ElemA->Get();
	});

	std::transform(WaitFor.begin(), WaitFor.end(), RawWaitForSemaphores.begin(), [](auto* ElemA) {
		return ElemA->Get();
	});

	VkSubmitInfo SubmitInfo = {};
	SubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	SubmitInfo.commandBufferCount = 1;
	SubmitInfo.pCommandBuffers = &mCommandBuffer;
	SubmitInfo.waitSemaphoreCount = static_cast<uint32_t>(RawWaitForSemaphores.size());
	SubmitInfo.pWaitSemaphores = RawWaitForSemaphores.data();
	SubmitInfo.signalSemaphoreCount = static_cast<uint32_t>(RawSignalSemaphores.size());
	SubmitInfo.pSignalSemaphores = RawSignalSemaphores.data();
	SubmitInfo.pWaitDstStageMask = reinterpret_cast<VkPipelineStageFlags*>(WaitStage.data());

	Assert(vkQueueSubmit(Queue, 1, &SubmitInfo, CustomFence ? CustomFence->Get() : nullptr) == VK_SUCCESS);
}
