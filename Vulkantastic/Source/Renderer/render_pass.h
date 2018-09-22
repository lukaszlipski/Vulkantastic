#pragma once
#include "image.h"

enum class AttachmentLoadOp
{
	DONT_CARE = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
	CLEAR = VK_ATTACHMENT_LOAD_OP_CLEAR,
	LOAD = VK_ATTACHMENT_LOAD_OP_LOAD
};

enum class AttachmentStoreOp
{
	DONT_CARE = VK_ATTACHMENT_STORE_OP_DONT_CARE,
	STORE = VK_ATTACHMENT_STORE_OP_STORE
};

struct ColorAttachment
{
	ImageLayout StartLayout = ImageLayout::UNDEFINED;
	ImageLayout EndLayout = ImageLayout::SHADER_READ;
	ImageFormat Format = ImageFormat::R8G8B8A8;
	AttachmentLoadOp LoadOp = AttachmentLoadOp::CLEAR;
	AttachmentStoreOp StoreOp = AttachmentStoreOp::STORE;
};

struct DepthAttachment
{
	ImageLayout StartLayout = ImageLayout::UNDEFINED;
	AttachmentLoadOp DepthLoadOp = AttachmentLoadOp::CLEAR;
	AttachmentStoreOp DepthStoreOp = AttachmentStoreOp::STORE;
	AttachmentLoadOp StencilLoadOp = AttachmentLoadOp::CLEAR;
	AttachmentStoreOp StencilStoreOp = AttachmentStoreOp::STORE;
};

class RenderPass
{
public:
	RenderPass(const std::vector<ColorAttachment>& Colors);
	RenderPass(const std::vector<ColorAttachment>& Colors, DepthAttachment Depth);
	~RenderPass();

	RenderPass(const RenderPass& Rhs) = delete;
	RenderPass& operator=(const RenderPass& Rhs) = delete;

	RenderPass(RenderPass&& Rhs) noexcept;
	RenderPass& operator=(RenderPass&& Rhs) noexcept;

	inline VkRenderPass GetRenderPass() const { return mRenderPass; }
	inline std::vector<ColorAttachment> GetColorAttachments() const { return mColorAttachments; }
	inline DepthAttachment GetDepthAttachment() const { return mDepthAttachment; }
	inline bool IsDepthEnabled() const { return mDepthEnabled; }

private:
	VkAttachmentDescription CreateColorAttachment(const ColorAttachment& AttachmentInfo) const;
	VkAttachmentDescription CreateDepthAttachment(const DepthAttachment& AttachmentInfo) const;
	void CreateRenderPass(const std::vector<VkAttachmentDescription>& Attachments, const std::vector<VkAttachmentReference>& AttachmentRefs, VkAttachmentReference* DepthAttachmentRef);

	VkRenderPass mRenderPass = nullptr;
	std::vector<ColorAttachment> mColorAttachments;
	DepthAttachment mDepthAttachment = {};
	bool mDepthEnabled = false;

};

