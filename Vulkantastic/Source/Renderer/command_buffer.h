#pragma once
#include "vulkan/vulkan_core.h"
#include <vector>
#include "pipeline_creation.h"
#include "synchronization.h"

enum class CBUsage
{
	ONE_TIME = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
	SIMULTANEOUS = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT
};

class CommandBuffer
{
	//class Semaphore;
	//class Fence;
public:
	CommandBuffer(int32_t QueueIndex);
	~CommandBuffer();

	void Begin(CBUsage Usage = CBUsage::ONE_TIME);
	void End();
	void Submit(Fence* CustomFence, std::vector<Semaphore*> Signal = {}, std::vector<Semaphore*> WaitFor = {}, std::vector<PipelineStage> WaitStage = {});
	void Submit(bool Wait = false, std::vector<Semaphore*> Signal = {}, std::vector<Semaphore*> WaitFor = {}, std::vector<PipelineStage> WaitStage = {});

	inline VkCommandBuffer GetCommandBuffer() const { return mCommandBuffer; }

private:

	VkCommandBuffer mCommandBuffer;
	int32_t mQueueIndex;

};