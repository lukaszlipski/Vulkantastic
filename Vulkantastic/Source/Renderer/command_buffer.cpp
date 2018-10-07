#define NOMINMAX
#include "command_buffer.h"
#include "../Utilities/assert.h"
#include "core.h"
#include "device.h"
#include <limits>

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

void CommandBuffer::Submit(bool Wait, std::vector<VkSemaphore> Signal, std::vector<VkSemaphore> WaitFor, std::vector<PipelineStage> WaitStage)
{
	VkFence WaitFence = nullptr;
	if (Wait)
	{
		const auto Device = VulkanCore::Get().GetDevice()->GetDevice();

		VkFenceCreateInfo FenceInfo = {};
		FenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

		Assert(vkCreateFence(Device, &FenceInfo, nullptr, &WaitFence) == VK_SUCCESS)
	}

	Submit(WaitFence, Signal, WaitFor, WaitStage);

	if (Wait)
	{
		const auto Device = VulkanCore::Get().GetDevice()->GetDevice();
		vkWaitForFences(Device, 1, &WaitFence, VK_TRUE, std::numeric_limits<uint64_t>::max());
	}

}

void CommandBuffer::Submit(VkFence CustomFence, std::vector<VkSemaphore> Signal, std::vector<VkSemaphore> WaitFor, std::vector<PipelineStage> WaitStage)
{
	const auto Queue = VulkanCore::Get().GetDevice()->GetQueueByIndex(mQueueIndex);

	VkSubmitInfo SubmitInfo = {};
	SubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	SubmitInfo.commandBufferCount = 1;
	SubmitInfo.pCommandBuffers = &mCommandBuffer;
	SubmitInfo.waitSemaphoreCount = static_cast<uint32_t>(WaitFor.size());
	SubmitInfo.pWaitSemaphores = WaitFor.data();
	SubmitInfo.signalSemaphoreCount = static_cast<uint32_t>(Signal.size());
	SubmitInfo.pSignalSemaphores = Signal.data();
	SubmitInfo.pWaitDstStageMask = reinterpret_cast<VkPipelineStageFlags*>(WaitStage.data());

	Assert(vkQueueSubmit(Queue, 1, &SubmitInfo, CustomFence) == VK_SUCCESS);
}
