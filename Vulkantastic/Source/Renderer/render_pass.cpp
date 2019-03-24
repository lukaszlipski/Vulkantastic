#include "render_pass.h"
#include "../Utilities/assert.h"


RenderPass::RenderPass(const std::vector<ColorAttachment>& Colors, DepthAttachment Depth)
	: mColorAttachments(Colors), mDepthAttachment(Depth), mDepthEnabled(true)
{
	std::vector<VkAttachmentDescription> Attachments;
	Attachments.reserve(mColorAttachments.size() + 1);
	std::vector<VkAttachmentReference> AttachmentRefs;
	AttachmentRefs.reserve(mColorAttachments.size() + 1);

	for (const auto & Color : mColorAttachments)
	{
		VkAttachmentDescription Attachment = CreateColorAttachment(Color);
		Attachments.push_back(Attachment);

		VkAttachmentReference ColorAttachmentRef = {};
		ColorAttachmentRef.attachment = static_cast<uint32_t>(AttachmentRefs.size());
		ColorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		AttachmentRefs.push_back(ColorAttachmentRef);
	}

	VkAttachmentDescription Attachment = CreateDepthAttachment(mDepthAttachment);
	Attachments.push_back(Attachment);	

	VkAttachmentReference DepthAttachmentRef = {};
	DepthAttachmentRef.attachment = static_cast<uint32_t>(AttachmentRefs.size());
	DepthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	CreateRenderPass(Attachments, AttachmentRefs, &DepthAttachmentRef);
}

RenderPass::RenderPass(const std::vector<ColorAttachment>& Colors)
	: mColorAttachments(Colors), mDepthEnabled(false)
{
	std::vector<VkAttachmentDescription> Attachments;
	Attachments.reserve(mColorAttachments.size());
	std::vector<VkAttachmentReference> AttachmentRefs;
	AttachmentRefs.reserve(mColorAttachments.size());

	for (const auto & Color : mColorAttachments)
	{
		VkAttachmentDescription Attachment = CreateColorAttachment(Color);
		Attachments.push_back(Attachment);

		VkAttachmentReference ColorAttachmentRef = {};
		ColorAttachmentRef.attachment = static_cast<uint32_t>(AttachmentRefs.size());
		ColorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		AttachmentRefs.push_back(ColorAttachmentRef);
	}

	CreateRenderPass(Attachments, AttachmentRefs, nullptr);
}

RenderPass::RenderPass(RenderPass&& Rhs) noexcept
{
	*this = std::move(Rhs);
}

VkAttachmentDescription RenderPass::CreateColorAttachment(const ColorAttachment& AttachmentInfo) const
{
	VkAttachmentDescription Result = {};

	Result.format = static_cast<VkFormat>(AttachmentInfo.Format);
	Result.initialLayout = static_cast<VkImageLayout>(AttachmentInfo.StartLayout);
	Result.finalLayout = static_cast<VkImageLayout>(AttachmentInfo.EndLayout);
	Result.loadOp = static_cast<VkAttachmentLoadOp>(AttachmentInfo.LoadOp);
	Result.storeOp = static_cast<VkAttachmentStoreOp>(AttachmentInfo.StoreOp);
	Result.samples = VK_SAMPLE_COUNT_1_BIT;
	Result.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	Result.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

	return Result;
}

VkAttachmentDescription RenderPass::CreateDepthAttachment(const DepthAttachment& AttachmentInfo) const
{
	VkAttachmentDescription Result = {};

	Result.format = VK_FORMAT_D24_UNORM_S8_UINT;
	Result.initialLayout = static_cast<VkImageLayout>(AttachmentInfo.StartLayout);
	Result.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	Result.loadOp = static_cast<VkAttachmentLoadOp>(AttachmentInfo.DepthLoadOp);
	Result.storeOp = static_cast<VkAttachmentStoreOp>(AttachmentInfo.DepthStoreOp);
	Result.samples = VK_SAMPLE_COUNT_1_BIT;
	Result.stencilLoadOp = static_cast<VkAttachmentLoadOp>(AttachmentInfo.StencilLoadOp);
	Result.stencilStoreOp = static_cast<VkAttachmentStoreOp>(AttachmentInfo.StencilStoreOp);

	return Result;
}

void RenderPass::CreateRenderPass(const std::vector<VkAttachmentDescription>& Attachments, const std::vector<VkAttachmentReference>& AttachmentRefs, VkAttachmentReference* DepthAttachmentRef)
{
	const auto Device = VulkanCore::Get().GetDevice()->GetDevice();

	VkSubpassDescription SubpassDesc = {};
	SubpassDesc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	SubpassDesc.colorAttachmentCount = static_cast<uint32_t>(AttachmentRefs.size());
	SubpassDesc.pColorAttachments = AttachmentRefs.data();
	SubpassDesc.pDepthStencilAttachment = DepthAttachmentRef;

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

RenderPass& RenderPass::operator=(RenderPass&& Rhs) noexcept
{
	mRenderPass = Rhs.mRenderPass;
	Rhs.mRenderPass = nullptr;

	mColorAttachments = std::move(Rhs.mColorAttachments);
	mDepthAttachment = Rhs.mDepthAttachment;
	mDepthEnabled = Rhs.mDepthEnabled;

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
