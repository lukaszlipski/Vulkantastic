#pragma once
#include <string>
#include "vulkan/vulkan_core.h"
#include "buffer.h"

enum class ImageUsage : uint8_t
{
	COLOR_ATTACHMENT = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
	DEPTH_ATTACHMENT = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
	SAMPLED = VK_IMAGE_USAGE_SAMPLED_BIT,
	TRANSFER_DST = VK_IMAGE_USAGE_TRANSFER_DST_BIT,
	TRANSFER_SRC = VK_IMAGE_USAGE_TRANSFER_SRC_BIT
};

inline ImageUsage operator|(ImageUsage Left, ImageUsage Right)
{
	return static_cast<ImageUsage>(static_cast<uint8_t>(Left) | static_cast<uint8_t>(Right));
}

inline ImageUsage operator&(ImageUsage Left, ImageUsage Right)
{
	return static_cast<ImageUsage>(static_cast<uint8_t>(Left) & static_cast<uint8_t>(Right));
}

enum class ImageFormat : uint8_t
{
	R8 = VK_FORMAT_R8_UNORM,
	R8_SRGB = VK_FORMAT_R8_SRGB,
	R8G8 = VK_FORMAT_R8G8_UNORM,
	R8G8_SRGB = VK_FORMAT_R8G8_SRGB,
	R8G8B8A8 = VK_FORMAT_R8G8B8A8_UNORM,
	R8G8B8A8_SRGB = VK_FORMAT_R8G8B8A8_SRGB,
	B8G8R8A8 = VK_FORMAT_B8G8R8A8_UNORM,
	D24S8 = VK_FORMAT_D24_UNORM_S8_UINT,
	BC1 = VK_FORMAT_BC1_RGB_UNORM_BLOCK,
	BC1_SRGB = VK_FORMAT_BC1_RGB_SRGB_BLOCK
};

enum class ImageType : uint8_t
{
	ONEDIM = VK_IMAGE_TYPE_1D,
	TWODIM = VK_IMAGE_TYPE_2D,
	TREEDIM = VK_IMAGE_TYPE_3D
};

enum class ImageLayout
{
	UNDEFINED = VK_IMAGE_LAYOUT_UNDEFINED,
	COLOR_ATTACHMENT = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
	DEPTH_STENCIL_ATTACHMENT = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
	SHADER_READ = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
	TRANSFER_SRC = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
	TRANSFER_DST = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
	PRESENT_SRC = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
};

struct ImageSettings
{
	ImageFormat Format = ImageFormat::R8G8B8A8;
	ImageType Type = ImageType::TWODIM;
	uint32_t Width = 1;
	uint32_t Height = 1;
	uint32_t Depth = 1;
	bool Mipmaps = true;
};

class Image
{
public:
	Image(std::vector<uint32_t> QueueIndices, ImageUsage Flags, bool GPUSide, ImageSettings Settings = {}, void* Data = nullptr);
	~Image();

	Image(const Image& Rhs) = delete;
	Image& operator=(const Image& Rhs) = delete;

	Image(Image&& Rhs) noexcept;
	Image& operator=(Image&& Rhs) noexcept;

	void UploadData(const void* Data, uint32_t Size);
	void ChangeLayout(ImageLayout DstLayout);
	void CopyFromBuffer(const Buffer* Other);
	void GenerateMipMaps();

	inline VkImage GetImage() const { return mImage; }
	inline uint64_t GetSize() const { return mAllocation.GetSize(); }
	inline ImageUsage GetFlags() const { return mFlags; }
	inline ImageLayout GetCurrentLayout() const { return mCurrentLayout; }
	inline uint32_t GetMipMapsCount() const { return mMipMapsCount; }
	inline ImageFormat GetFormat() const { return mSettings.Format; }

	static uint8_t GetNumComponentsByFormat(ImageFormat Format);
	static int32_t GetSizeInBytesByFormat(ImageFormat Format);

	static void ChangeMultipleLayouts(std::vector<Image*> Images, std::vector<ImageLayout> Layouts);

private:
	VkImage mImage = nullptr;
	ImageLayout mCurrentLayout = ImageLayout::UNDEFINED;
	uint32_t mMipMapsCount = 1;

	Allocation mAllocation{};
	std::vector<uint32_t> mQueueIndices;
	ImageUsage mFlags;
	VkMemoryRequirements mMemoryRequirements;
	ImageSettings mSettings = {};
	bool mGPUSide;

};

using upImage = std::unique_ptr<Image>;