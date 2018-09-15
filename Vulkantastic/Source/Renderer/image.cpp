#define NOMINMAX
#include "image.h"
#include <algorithm>
#include "../Utilities/assert.h"
#include "command_buffer.h"

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

	const int32_t GraphicsIndex = VulkanCore::Get().GetDevice()->GetQueuesIndicies().GraphicsIndex;

	CommandBuffer Cb(GraphicsIndex);
	Cb.Begin();

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
	vkCmdPipelineBarrier(Cb.GetCommandBuffer(), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &Transition);

	Cb.End();
	Cb.Submit(true);


	mCurrentLayout = DstLayout;
}

void Image::CopyFromBuffer(const Buffer* Other)
{
	auto CurrentLayoutCpy = mCurrentLayout;
	ChangeLayout(ImageLayout::TRANSFER_DST);

	Assert((GetFlags() & ImageUsage::TRANSFER_DST) == ImageUsage::TRANSFER_DST);
	Assert((Other->GetFlags() & BufferUsage::TRANSFER_SRC) == BufferUsage::TRANSFER_SRC);

	const int32_t GraphicsIndex = VulkanCore::Get().GetDevice()->GetQueuesIndicies().GraphicsIndex;

	CommandBuffer Cb(GraphicsIndex);
	Cb.Begin();

	VkBufferImageCopy CopyInfo = {};
	CopyInfo.imageExtent.depth = 1;
	CopyInfo.imageExtent.height = mSettings.Height;
	CopyInfo.imageExtent.width = mSettings.Width;
	CopyInfo.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	CopyInfo.imageSubresource.layerCount = 1;
	CopyInfo.imageSubresource.mipLevel = 0;

	vkCmdCopyBufferToImage(Cb.GetCommandBuffer(), Other->GetBuffer(), mImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &CopyInfo);

	Cb.End();
	Cb.Submit(true);

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
