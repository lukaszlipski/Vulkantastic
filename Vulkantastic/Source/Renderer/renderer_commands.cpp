#define NOMINMAX
#include "renderer_commands.h"
#include "shader_parameters.h"

void Cmd::BeginRenderPass(CommandBuffer* Cb, Framebuffer* Fb, RenderPass* Rp, const std::vector<VkClearValue>& ClearColors, VkExtent2D Extend)
{
	VkRenderPassBeginInfo RenderPassBeginInfo = {};
	RenderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	RenderPassBeginInfo.framebuffer = Fb->GetFramebuffer();
	RenderPassBeginInfo.renderPass = Rp->GetRenderPass();
	RenderPassBeginInfo.renderArea.offset = { 0,0 };
	RenderPassBeginInfo.renderArea.extent = Extend;
	RenderPassBeginInfo.clearValueCount = static_cast<uint32_t>(ClearColors.size());
	RenderPassBeginInfo.pClearValues = ClearColors.data();

	vkCmdBeginRenderPass(Cb->GetCommandBuffer(), &RenderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void Cmd::EndRenderPass(CommandBuffer* Cb)
{
	vkCmdEndRenderPass(Cb->GetCommandBuffer());
}

void Cmd::BindGraphicsPipeline(CommandBuffer* Cb, IGraphicsPipeline* Pipeline)
{
	vkCmdBindPipeline(Cb->GetCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, Pipeline->GetPipeline());
}

void Cmd::BindVertexBuffer(CommandBuffer* Cb, Buffer* VertexBuffer)
{
	VkDeviceSize Offsets[] = { 0 };
	auto BufferTmp = VertexBuffer->GetBuffer();
	vkCmdBindVertexBuffers(Cb->GetCommandBuffer(), 0, 1, &BufferTmp, Offsets);
}

void Cmd::BindVertexAndIndexBuffer(CommandBuffer* Cb, Buffer* VertexBuffer, Buffer* IndexBuffer)
{
	BindVertexBuffer(Cb, VertexBuffer);

	vkCmdBindIndexBuffer(Cb->GetCommandBuffer(), IndexBuffer->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);
}

void Cmd::DrawIndexed(CommandBuffer* Cb, uint32_t Size, uint32_t InstancesCount /*= 1*/)
{
	vkCmdDrawIndexed(Cb->GetCommandBuffer(), Size, InstancesCount, 0, 0, 0);
}

void Cmd::Draw(CommandBuffer* Cb, uint32_t Size, uint32_t InstancesCount /*= 1*/)
{
	vkCmdDraw(Cb->GetCommandBuffer(), Size, InstancesCount, 0, 0);
}

void Cmd::UpdateDescriptorData(CommandBuffer* Cb, ShaderParameters* Data, DescriptorInst* DescSet, IPipeline* Pipeline, std::vector<uint32_t> DynamicOffsets)
{
	auto PCVertPtr = Data->GetPushConstantBuffer(ShaderType::VERTEX);
	if (PCVertPtr)
	{
		vkCmdPushConstants(Cb->GetCommandBuffer(), Pipeline->GetPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT, PCVertPtr->GetOffset(), PCVertPtr->GetSize(), PCVertPtr->GetBuffer());
	}

	auto PCFragPtr = Data->GetPushConstantBuffer(ShaderType::FRAGMENT);
	if (PCFragPtr)
	{
		vkCmdPushConstants(Cb->GetCommandBuffer(), Pipeline->GetPipelineLayout(), VK_SHADER_STAGE_FRAGMENT_BIT, PCFragPtr->GetOffset(), PCFragPtr->GetSize(), PCFragPtr->GetBuffer());
	}

	auto Set = DescSet->GetSet();
	vkCmdBindDescriptorSets(Cb->GetCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, Pipeline->GetPipelineLayout(), DescSet->GetSetIndex(), 1, &Set, static_cast<uint32_t>(DynamicOffsets.size()), DynamicOffsets.data());
}

void Cmd::SetViewports(CommandBuffer* Cb, IGraphicsPipeline* Pipeline)
{
	auto Viewports = Pipeline->GetViewports();
	vkCmdSetViewport(Cb->GetCommandBuffer(), 0, static_cast<uint32_t>(Viewports.size()), Viewports.data());
}

void Cmd::ChangeLayout(CommandBuffer* Cb, Image* Img, ImageLayout DstLayout)
{
	VkImageMemoryBarrier Transition = {};
	Transition.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	Transition.image = Img->GetImage();
	Transition.oldLayout = static_cast<VkImageLayout>(Img->GetCurrentLayout());
	Transition.newLayout = static_cast<VkImageLayout>(DstLayout);
	Transition.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	Transition.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	Transition.subresourceRange.aspectMask = Img->GetFormat() == ImageFormat::D24S8 ? VK_IMAGE_ASPECT_STENCIL_BIT | VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
	Transition.subresourceRange.baseArrayLayer = 0;
	Transition.subresourceRange.baseMipLevel = 0;
	Transition.subresourceRange.layerCount = 1;
	Transition.subresourceRange.levelCount = Img->GetMipMapsCount();

	// #TODO: Select proper mask and flags based on current and destination layout
	Transition.srcAccessMask = 0;
	Transition.dstAccessMask = 0;
	vkCmdPipelineBarrier(Cb->GetCommandBuffer(), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &Transition);
}

void QueuePresent(uint32_t ImageIndex, Semaphore* ImageReadyToPresent)
{
	VkPresentInfoKHR PresentInfo = {};
	PresentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	PresentInfo.swapchainCount = 1;
	auto SwapChain = VulkanCore::Get().GetSwapChain()->GetSwapChain();
	PresentInfo.pSwapchains = &SwapChain;
	PresentInfo.waitSemaphoreCount = 1;
	PresentInfo.pWaitSemaphores = ImageReadyToPresent->GetPtr();
	PresentInfo.pImageIndices = &ImageIndex;

	vkQueuePresentKHR(VulkanCore::Get().GetDevice()->GetGraphicsQueue(), &PresentInfo);
}

uint32_t AcquireNextImage(Semaphore* ImageReadyToDraw)
{
	auto Device = VulkanCore::Get().GetDevice()->GetDevice();
	uint32_t ImageIndex;
	vkAcquireNextImageKHR(Device, VulkanCore::Get().GetSwapChain()->GetSwapChain(), std::numeric_limits<uint64_t>::max(), ImageReadyToDraw->Get(), VK_NULL_HANDLE, &ImageIndex);
	return ImageIndex;
}
