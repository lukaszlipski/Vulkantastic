#include "framebuffer.h"
#include <algorithm>
#include "../Utilities/assert.h"

Framebuffer::Framebuffer(const std::vector<ImageView*>& Views, const RenderPass& RPass, float Width, float Height)
{
	const auto Device = VulkanCore::Get().GetDevice()->GetDevice();

	std::vector<VkImageView> ImageViews;

	std::for_each(Views.begin(), Views.end(), [&ImageViews](const auto& Elem) {
		ImageViews.push_back(Elem->GetView());
	});

	VkFramebufferCreateInfo Info = {};
	Info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	Info.attachmentCount = ImageViews.size();
	Info.pAttachments = ImageViews.data();
	Info.layers = 1;
	Info.renderPass = RPass.GetRenderPass();
	Info.height = Height;
	Info.width = Width;
	
	Assert(vkCreateFramebuffer(Device, &Info, nullptr, &mFramebuffer) == VK_SUCCESS);

}

Framebuffer::Framebuffer(Framebuffer&& Rhs) noexcept
{
	*this = std::move(Rhs);
}

Framebuffer& Framebuffer::operator=(Framebuffer&& Rhs) noexcept
{
	mFramebuffer = Rhs.mFramebuffer;
	Rhs.mFramebuffer = nullptr;

	return *this;
}

Framebuffer::~Framebuffer()
{
	if (mFramebuffer)
	{
		const auto Device = VulkanCore::Get().GetDevice()->GetDevice();

		vkDestroyFramebuffer(Device, mFramebuffer, nullptr);
	}
}
