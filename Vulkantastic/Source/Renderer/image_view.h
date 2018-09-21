#pragma once
#include "vulkan/vulkan_core.h"
#include "image.h"


struct ImageViewSettings
{
	uint32_t BaseMipMapLevel = 0;
	uint32_t MipMapLevelCount = 1;
	uint32_t BaseArrayLevel = 0;
	uint32_t ArrayLevelCount = 1;
	ImageFormat Format = ImageFormat::R8G8B8A8;
};


class ImageView
{
public:
	ImageView(Image* DesiredImage, ImageViewSettings Settings);
	~ImageView();

	ImageView(const ImageView& Rhs) = delete;
	ImageView& operator=(const ImageView& Rhs) = delete;

	ImageView(ImageView&& Rhs) noexcept;
	ImageView& operator=(ImageView&& Rhs) noexcept;

	inline VkImageView GetView() const { return mView; }

private:
	VkImageView mView = nullptr;
	Image* mImage;
	ImageViewSettings mSettings;

};

