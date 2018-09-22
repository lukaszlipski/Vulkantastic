#include "render_pass.h"
#include "../Utilities/assert.h"


RenderPass::RenderPass(const std::vector<ColorAttachment>& Colors, DepthAttachment Depth)
{
	const auto Device = VulkanCore::Get().GetDevice()->GetDevice();

	std::vector<VkAttachmentDescription> Attachments;
	Attachments.resize(Colors.size());
	std::vector<VkAttachmentReference> AttachmentRefs;
	AttachmentRefs.reserve(Colors.size());

	auto Attachment = Attachments.begin();
	auto AttachmentRef = AttachmentRefs.begin();
	for (auto Color = Colors.begin(); Color != Colors.end(); ++Color, ++Attachment)
	{
		Attachment->format = static_cast<VkFormat>(Color->Format);
		Attachment->initialLayout = static_cast<VkImageLayout>(Color->StartLayout);
		Attachment->finalLayout = static_cast<VkImageLayout>(Color->EndLayout);
		Attachment->loadOp = static_cast<VkAttachmentLoadOp>(Color->LoadOp);
		Attachment->storeOp = static_cast<VkAttachmentStoreOp>(Color->StoreOp);
		Attachment->samples = VK_SAMPLE_COUNT_1_BIT;
		Attachment->stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		Attachment->stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

		VkAttachmentReference ColorAttachmentRef = {};
		ColorAttachmentRef.attachment = static_cast<uint32_t>(AttachmentRefs.size());
		ColorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		AttachmentRefs.push_back(ColorAttachmentRef);
	}


	VkAttachmentReference DepthAttachmentRef = {};

	if (Depth.Enable)
	{
		VkAttachmentDescription Attachment = {};
		Attachment.format = VK_FORMAT_D24_UNORM_S8_UINT;
		Attachment.initialLayout = static_cast<VkImageLayout>(Depth.StartLayout);
		Attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		Attachment.loadOp = static_cast<VkAttachmentLoadOp>(Depth.DepthLoadOp);
		Attachment.storeOp = static_cast<VkAttachmentStoreOp>(Depth.DepthStoreOp);
		Attachment.samples = VK_SAMPLE_COUNT_1_BIT;
		Attachment.stencilLoadOp = static_cast<VkAttachmentLoadOp>(Depth.StencilLoadOp);
		Attachment.stencilStoreOp = static_cast<VkAttachmentStoreOp>(Depth.StencilStoreOp);

		Attachments.push_back(Attachment);

		DepthAttachmentRef.attachment = static_cast<uint32_t>(AttachmentRefs.size());
		DepthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	}

	VkSubpassDescription SubpassDesc = {};
	SubpassDesc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	SubpassDesc.colorAttachmentCount = 1;
	SubpassDesc.pColorAttachments = AttachmentRefs.data();
	SubpassDesc.pDepthStencilAttachment = Depth.Enable ? &DepthAttachmentRef : nullptr;

	VkSubpassDependency SubpassDependency = {};
	SubpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	SubpassDependency.dstSubpass = 0;
	SubpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	SubpassDependency.srcAccessMask = 0;
	SubpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	SubpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkRenderPassCreateInfo RenderPassInfo = {};
	RenderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	RenderPassInfo.subpassCount = 1;
	RenderPassInfo.pSubpasses = &SubpassDesc;
	RenderPassInfo.attachmentCount = static_cast<uint32_t>(Attachments.size());
	RenderPassInfo.pAttachments = Attachments.data();
	RenderPassInfo.dependencyCount = 1;
	RenderPassInfo.pDependencies = &SubpassDependency;

	Assert(vkCreateRenderPass(Device, &RenderPassInfo, nullptr, &mRenderPass) == VK_SUCCESS);

}

RenderPass::RenderPass(RenderPass&& Rhs) noexcept
{
	*this = std::move(Rhs);
}

RenderPass& RenderPass::operator=(RenderPass&& Rhs) noexcept
{
	mRenderPass = Rhs.mRenderPass;
	Rhs.mRenderPass = nullptr;

	return *this;
}

RenderPass::~RenderPass()
{
	if (mRenderPass)
	{
		const auto Device = VulkanCore::Get().GetDevice()->GetDevice();

		vkDestroyRenderPass(Device, mRenderPass, nullptr);
	}
}
