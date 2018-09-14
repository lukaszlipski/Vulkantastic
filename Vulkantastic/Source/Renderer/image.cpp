#define NOMINMAX
#include "image.h"
#include <algorithm>
#include "../Utilities/assert.h"

Image::Image(std::initializer_list<uint32_t> QueueIndices, ImageUsage Flags, bool GPUSide, ImageSettings Settings /* = {} */, void* Data /* = nullptr */)
	: mQueueIndices(QueueIndices), mFlags(Flags), mGPUSide(GPUSide), mSettings(Settings)
{
	
	VkImageCreateInfo ImageInfo = {};
	ImageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	ImageInfo.arrayLayers = 1;
	ImageInfo.extent.width = Settings.Width;
	ImageInfo.extent.height = Settings.Height;
	ImageInfo.extent.depth = Settings.Depth;

	if (Settings.Mipmaps)
	{
		mMipMapsCount = static_cast<uint32_t>(std::floor(std::log2(std::max(Settings.Width, Settings.Height))) + 1);
	}
	
	ImageInfo.mipLevels = mMipMapsCount;
	ImageInfo.format = static_cast<VkFormat>(Settings.Format);
	ImageInfo.imageType = static_cast<VkImageType>(Settings.Type);
	ImageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	if (mQueueIndices.size() == 1)
	{
		ImageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}
	else
	{
		ImageInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
		ImageInfo.queueFamilyIndexCount = static_cast<uint32_t>(mQueueIndices.size());
		ImageInfo.pQueueFamilyIndices = mQueueIndices.data();
	}

	ImageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	ImageInfo.usage = static_cast<VkImageUsageFlags>(Flags);
	ImageInfo.samples = VK_SAMPLE_COUNT_1_BIT;

	const auto Device = VulkanCore::Get().GetDevice()->GetDevice();

	Assert(vkCreateImage(Device, &ImageInfo, nullptr, &mImage) == VK_SUCCESS);

	vkGetImageMemoryRequirements(Device, mImage, &mMemoryRequirements);
	MemoryManager::Get().Allocate(mAllocation, mMemoryRequirements, mGPUSide);
	Assert(vkBindImageMemory(Device, mImage, mAllocation.GetMemory(), mAllocation.GetOffset()) == VK_SUCCESS);

	UploadData(Data, mSettings.Width * mSettings.Height * GetNumComponentsByFormat(mSettings.Format));

}

Image::~Image()
{
	const auto Device = VulkanCore::Get().GetDevice()->GetDevice();

	MemoryManager::Get().Free(mAllocation);
	vkDestroyImage(Device, mImage, nullptr);
}

void Image::UploadData(const void* Data, uint32_t Size)
{
	const bool IsDataValid = Data != nullptr;
	if (!IsDataValid) { return; }

	const bool IsSizeValid = Size == (mSettings.Height * mSettings.Width * GetNumComponentsByFormat(mSettings.Format));
	Assert(IsSizeValid);

	uint32_t GraphicsQueueIndex = VulkanCore::Get().GetDevice()->GetQueuesIndicies().GraphicsIndex;
	Buffer Tmp({ GraphicsQueueIndex }, BufferUsage::TRANSFER_SRC, false, Size, Data);

	CopyFromBuffer(&Tmp);
}

void Image::ChangeLayout(ImageLayout DstLayout)
{
	if (DstLayout == mCurrentLayout) { return; }

	const auto Device = VulkanCore::Get().GetDevice()->GetDevice();
	const VkCommandPool CommandPool = VulkanCore::Get().GetGraphicsCommandPoolForCurrentThread();
	const VkQueue GraphicsQueue = VulkanCore::Get().GetDevice()->GetGraphicsQueue();

	VkCommandBufferAllocateInfo CommandBufferAllocateInfo = {};
	CommandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	CommandBufferAllocateInfo.commandBufferCount = 1;
	CommandBufferAllocateInfo.commandPool = CommandPool;
	CommandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

	VkCommandBuffer ChangeLayoutCommandBuffer;

	Assert(vkAllocateCommandBuffers(Device, &CommandBufferAllocateInfo, &ChangeLayoutCommandBuffer) == VK_SUCCESS);

	VkCommandBufferBeginInfo ChangeLayoutCommandBeginInfo = {};
	ChangeLayoutCommandBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	ChangeLayoutCommandBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	Assert(vkBeginCommandBuffer(ChangeLayoutCommandBuffer, &ChangeLayoutCommandBeginInfo) == VK_SUCCESS);

	VkImageMemoryBarrier Transition = {};
	Transition.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	Transition.image = mImage;
	Transition.oldLayout = static_cast<VkImageLayout>(mCurrentLayout);
	Transition.newLayout = static_cast<VkImageLayout>(DstLayout);
	Transition.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	Transition.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	Transition.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	Transition.subresourceRange.baseArrayLayer = 0;
	Transition.subresourceRange.baseMipLevel = 0;
	Transition.subresourceRange.layerCount = 1;
	Transition.subresourceRange.levelCount = mMipMapsCount;
	
	// #TODO: Select proper mask and flags based on current and destination layout
	Transition.srcAccessMask = 0;
	Transition.dstAccessMask = 0;
	vkCmdPipelineBarrier(ChangeLayoutCommandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &Transition);

	vkEndCommandBuffer(ChangeLayoutCommandBuffer);

	VkSubmitInfo SubmitInfo = {};
	SubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	SubmitInfo.commandBufferCount = 1;
	SubmitInfo.pCommandBuffers = &ChangeLayoutCommandBuffer;

	Assert(vkQueueSubmit(GraphicsQueue, 1, &SubmitInfo, VK_NULL_HANDLE) == VK_SUCCESS);
	vkQueueWaitIdle(GraphicsQueue);

	vkFreeCommandBuffers(Device, CommandPool, 1, &ChangeLayoutCommandBuffer);

	mCurrentLayout = DstLayout;
}

void Image::CopyFromBuffer(const Buffer* Other)
{
	auto CurrentLayoutCpy = mCurrentLayout;
	ChangeLayout(ImageLayout::TRANSFER_DST);

	Assert((GetFlags() & ImageUsage::TRANSFER_DST) == ImageUsage::TRANSFER_DST);
	Assert((Other->GetFlags() & BufferUsage::TRANSFER_SRC) == BufferUsage::TRANSFER_SRC);

	const auto Device = VulkanCore::Get().GetDevice()->GetDevice();
	const VkCommandPool CommandPool = VulkanCore::Get().GetGraphicsCommandPoolForCurrentThread();
	const VkQueue GraphicsQueue = VulkanCore::Get().GetDevice()->GetGraphicsQueue();

	VkCommandBufferAllocateInfo CommandBufferAllocateInfo = {};
	CommandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	CommandBufferAllocateInfo.commandBufferCount = 1;
	CommandBufferAllocateInfo.commandPool = CommandPool;
	CommandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

	VkCommandBuffer CopyCommandBuffer;

	Assert(vkAllocateCommandBuffers(Device, &CommandBufferAllocateInfo, &CopyCommandBuffer) == VK_SUCCESS);

	VkCommandBufferBeginInfo CopyCommandBeginInfo = {};
	CopyCommandBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	CopyCommandBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	Assert(vkBeginCommandBuffer(CopyCommandBuffer, &CopyCommandBeginInfo) == VK_SUCCESS);

	VkBufferImageCopy CopyInfo = {};
	CopyInfo.imageExtent.depth = 1;
	CopyInfo.imageExtent.height = mSettings.Height;
	CopyInfo.imageExtent.width = mSettings.Width;
	CopyInfo.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	CopyInfo.imageSubresource.layerCount = 1;
	CopyInfo.imageSubresource.mipLevel = 0;

	vkCmdCopyBufferToImage(CopyCommandBuffer, Other->GetBuffer(), mImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &CopyInfo);

	vkEndCommandBuffer(CopyCommandBuffer);

	VkSubmitInfo SubmitInfo = {};
	SubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	SubmitInfo.commandBufferCount = 1;
	SubmitInfo.pCommandBuffers = &CopyCommandBuffer;

	vkQueueSubmit(GraphicsQueue, 1, &SubmitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(GraphicsQueue);

	vkFreeCommandBuffers(Device, CommandPool, 1, &CopyCommandBuffer);

	if (CurrentLayoutCpy != ImageLayout::UNDEFINED)
	{
		ChangeLayout(CurrentLayoutCpy);
	}

}

uint8_t Image::GetNumComponentsByFormat(ImageFormat Format)
{
	switch (Format)
	{
	case ImageFormat::R8:
		return 1;
	case ImageFormat::R8G8:
		return 2;
	case ImageFormat::R8G8B8A8:
		return 4;
	}
	return 0;
}
