#include "buffer.h"
#include "core.h"
#include "device.h"
#include "../Utilities/assert.h"

Buffer::Buffer(std::initializer_list<uint32_t> QueueIndices, BufferUsage Flags, bool GPUSide, uint32_t Size, const void* Data /*= nullptr*/) 
	: mQueueIndices(QueueIndices), mFlags(Flags), mGPUSide(GPUSide), mSize(Size)
{
	VkBufferCreateInfo BufferInfo = {};
	BufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	BufferInfo.usage = static_cast<VkBufferUsageFlags>(mFlags);

	BufferInfo.size = mSize;

	if (mQueueIndices.size() == 1)
	{
		BufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}
	else
	{
		BufferInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
		BufferInfo.queueFamilyIndexCount = static_cast<uint32_t>(mQueueIndices.size());
		BufferInfo.pQueueFamilyIndices = mQueueIndices.data();
	}

	const auto Device = VulkanCore::Get().GetDevice()->GetDevice();

	Assert(vkCreateBuffer(Device, &BufferInfo, nullptr, &mBuffer) == VK_SUCCESS);
	vkGetBufferMemoryRequirements(Device, mBuffer, &mMemoryRequirements);
	MemoryManager::Get().Allocate(mAllocation, mMemoryRequirements, mGPUSide);
	Assert(vkBindBufferMemory(Device, mBuffer, mAllocation.GetMemory(), mAllocation.GetOffset()) == VK_SUCCESS);

	UploadData(Data, mSize, 0);
}

Buffer::~Buffer()
{
	const auto Device = VulkanCore::Get().GetDevice()->GetDevice();

	MemoryManager::Get().Free(mAllocation);
	vkDestroyBuffer(Device, mBuffer, nullptr);
}

void Buffer::UploadData(const void* Data, uint32_t Size, uint32_t Offset /*= 0*/)
{
	if (Data)
	{
		if (mGPUSide)
		{
			uint32_t GraphicsQueueIndex = VulkanCore::Get().GetDevice()->GetQueuesIndicies().GraphicsIndex;
			Buffer Tmp({ GraphicsQueueIndex }, BufferUsage::TRANSFER_SRC, false, mSize, Data);

			CopyFromBuffer(&Tmp, mSize, 0, 0);
		}
		else
		{
			MemoryManager::Get().UploadData(mAllocation, Data, Size, Offset);
		}
	}
}

void Buffer::CopyFromBuffer(const Buffer* Other, uint64_t Size, uint64_t SrcOffset /*= 0*/, uint64_t DstOffset /*= 0*/)
{
	Assert((GetFlags() & BufferUsage::TRANSFER_DST) == BufferUsage::TRANSFER_DST);
	Assert((Other->GetFlags() & BufferUsage::TRANSFER_SRC) == BufferUsage::TRANSFER_SRC);

	const auto Device = VulkanCore::Get().GetDevice()->GetDevice();
	const VkCommandPool CommandPool = VulkanCore::Get().GetGraphicsCommandPoolForCurrentThread();
	const VkQueue GraphicsQueue = VulkanCore::Get().GetDevice()->GetGraphicsQueue();

	VkCommandBuffer CopyCommandBuffer;

	VkCommandBufferAllocateInfo CommandBufferAllocateInfo = {};
	CommandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	CommandBufferAllocateInfo.commandBufferCount = 1;
	CommandBufferAllocateInfo.commandPool = CommandPool;
	CommandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

	Assert(vkAllocateCommandBuffers(Device, &CommandBufferAllocateInfo, &CopyCommandBuffer) == VK_SUCCESS);

	VkCommandBufferBeginInfo CopyCommandBeginInfo = {};
	CopyCommandBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	CopyCommandBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	VkBufferCopy CopyInfo = {};
	CopyInfo.size = Size;
	CopyInfo.dstOffset = DstOffset;
	CopyInfo.srcOffset = SrcOffset;

	vkBeginCommandBuffer(CopyCommandBuffer, &CopyCommandBeginInfo);
	vkCmdCopyBuffer(CopyCommandBuffer, Other->GetBuffer(), GetBuffer(), 1, &CopyInfo);
	vkEndCommandBuffer(CopyCommandBuffer);

	// Invoke copy
	VkSubmitInfo CopySubmitInfo = {};
	CopySubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	CopySubmitInfo.commandBufferCount = 1;
	CopySubmitInfo.pCommandBuffers = &CopyCommandBuffer;

	Assert(vkQueueSubmit(GraphicsQueue, 1, &CopySubmitInfo, VK_NULL_HANDLE) == VK_SUCCESS);
	vkQueueWaitIdle(GraphicsQueue);

	vkFreeCommandBuffers(Device, CommandPool, 1, &CopyCommandBuffer);

}
