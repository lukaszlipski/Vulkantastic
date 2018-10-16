#pragma once
#include "vulkan/vulkan_core.h"
#include "render_pass.h"
#include "image_view.h"

class Framebuffer
{
public:
	Framebuffer(const std::vector<ImageView*>& Views, const RenderPass& RPass, float Width, float Height);
	~Framebuffer();

	Framebuffer(const Framebuffer& Rhs) = delete;
	Framebuffer& operator=(const Framebuffer& Rhs) = delete;

	Framebuffer(Framebuffer&& Rhs) noexcept;
	Framebuffer& operator=(Framebuffer&& Rhs) noexcept;

	inline VkFramebuffer GetFramebuffer() const { return mFramebuffer; }

private:
	VkFramebuffer mFramebuffer = nullptr;

};