#pragma once
#include "vulkan/vulkan_core.h"
#include "../Renderer/command_buffer.h"
#include "../Renderer/framebuffer.h"
#include "../Renderer/device.h"
#include "../Renderer/swap_chain.h"
#include "../Renderer/pipeline.h"
#include "../Renderer/push_constant_buffer.h"

void CmdBeginRenderPass(CommandBuffer* Cb, Framebuffer* Fb, RenderPass* Rp, const std::vector<VkClearValue>& ClearColors, VkExtent2D Extend)
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

void CmdEndRenderPass(CommandBuffer* Cb)
{
	vkCmdEndRenderPass(Cb->GetCommandBuffer());
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

void CmdBindGraphicsPipeline(CommandBuffer* Cb, IGraphicsPipeline* Pipeline)
{
	vkCmdBindPipeline(Cb->GetCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, Pipeline->GetPipeline());
}

void CmdBindVertexAndIndexBuffer(CommandBuffer* Cb, Buffer* VertexBuffer, Buffer* IndexBuffer)
{
	VkDeviceSize Offsets[] = { 0 };
	auto BufferTmp = VertexBuffer->GetBuffer();
	vkCmdBindVertexBuffers(Cb->GetCommandBuffer(), 0, 1, &BufferTmp, Offsets);

	vkCmdBindIndexBuffer(Cb->GetCommandBuffer(), IndexBuffer->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);
}

void CmdDrawIndexed(CommandBuffer* Cb, uint32_t Size, uint32_t InstancesCount = 1)
{
	vkCmdDrawIndexed(Cb->GetCommandBuffer(), Size, InstancesCount, 0, 0, 0);
}

void CmdUpdateDescriptorData(CommandBuffer* Cb, DescriptorInst* Data, IPipeline* Pipeline)
{
	auto PCVertPtr = Data->GetPushConstantBuffer(ShaderType::VERTEX);
	vkCmdPushConstants(Cb->GetCommandBuffer(), Pipeline->GetPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT, PCVertPtr->GetOffset(), PCVertPtr->GetSize(), PCVertPtr->GetBuffer());

	auto PCFragPtr = Data->GetPushConstantBuffer(ShaderType::FRAGMENT);
	vkCmdPushConstants(Cb->GetCommandBuffer(), Pipeline->GetPipelineLayout(), VK_SHADER_STAGE_FRAGMENT_BIT, PCFragPtr->GetOffset(), PCFragPtr->GetSize(), PCFragPtr->GetBuffer());

	auto Set = Data->GetSet();
	vkCmdBindDescriptorSets(Cb->GetCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, Pipeline->GetPipelineLayout(), 0, 1, &Set, 0, nullptr);
}

void CmdSetViewports(CommandBuffer* Cb, IGraphicsPipeline* Pipeline)
{
	auto Viewports = Pipeline->GetViewports();
	vkCmdSetViewport(Cb->GetCommandBuffer(), 0, static_cast<uint32_t>(Viewports.size()), Viewports.data());
}

uint32_t AcquireNextImage(Semaphore* ImageReadyToDraw)
{
	auto Device = VulkanCore::Get().GetDevice()->GetDevice();
	uint32_t ImageIndex;
	vkAcquireNextImageKHR(Device, VulkanCore::Get().GetSwapChain()->GetSwapChain(), std::numeric_limits<uint64_t>::max(), ImageReadyToDraw->Get(), VK_NULL_HANDLE, &ImageIndex);
	return ImageIndex;
}
