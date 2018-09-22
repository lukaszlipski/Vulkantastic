#include "image_view.h"
#include "../Utilities/assert.h"


ImageView::ImageView(Image* DesiredImage, ImageViewSettings Settings)
	: mImage(DesiredImage), mSettings(Settings)
{
	Assert(mImage != nullptr);

	const auto Device = VulkanCore::Get().GetDevice()->GetDevice();

	VkImageViewCreateInfo ViewInfo = {};
	ViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	ViewInfo.format = static_cast<VkFormat>(mSettings.Format);
	ViewInfo.image = DesiredImage->GetImage();
	ViewInfo.subresourceRange.baseMipLevel = mSettings.BaseArrayLevel;
	ViewInfo.subresourceRange.levelCount = mSettings.MipMapLevelCount;
	ViewInfo.subresourceRange.baseArrayLayer = mSettings.BaseArrayLevel;
	ViewInfo.subresourceRange.layerCount = mSettings.ArrayLevelCount;
	ViewInfo.subresourceRange.aspectMask = mSettings.Format == ImageFormat::D24S8 ? VK_IMAGE_ASPECT_STENCIL_BIT | VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
	ViewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	ViewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	ViewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	ViewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	ViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;

	vkCreateImageView(Device, &ViewInfo, nullptr, &mView);

}

ImageView::ImageView(ImageView&& Rhs) noexcept
{
	*this = std::move(Rhs);
}

ImageView& ImageView::operator=(ImageView&& Rhs) noexcept
{
	mView = Rhs.mView;
	mSettings = Rhs.mSettings;
	Rhs.mView = nullptr;

	return *this;
}

ImageView::~ImageView()
{
	if (!mView) { return; }

	const auto Device = VulkanCore::Get().GetDevice()->GetDevice();

	vkDestroyImageView(Device, mView, nullptr);
}
