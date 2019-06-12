#pragma once
#include "vulkan/vulkan_core.h"
#include "../Renderer/command_buffer.h"
#include "../Renderer/framebuffer.h"
#include "../Renderer/device.h"
#include "../Renderer/swap_chain.h"
#include "../Renderer/pipeline.h"
#include "../Renderer/uniform_raw_data.h"


namespace Cmd
{
	void BeginRenderPass(CommandBuffer* Cb, Framebuffer* Fb, RenderPass* Rp, const std::vector<VkClearValue>& ClearColors, VkExtent2D Extend);

	void EndRenderPass(CommandBuffer* Cb);

	void BindGraphicsPipeline(CommandBuffer* Cb, IGraphicsPipeline* Pipeline);

	void BindVertexBuffer(CommandBuffer* Cb, Buffer* VertexBuffer);

	void BindVertexAndIndexBuffer(CommandBuffer* Cb, Buffer* VertexBuffer, Buffer* IndexBuffer);

	void DrawIndexed(CommandBuffer* Cb, uint32_t Size, uint32_t InstancesCount = 1);

	void Draw(CommandBuffer* Cb, uint32_t Size, uint32_t InstancesCount = 1);

	void UpdateDescriptorData(CommandBuffer* Cb, ShaderParameters* Data, DescriptorInst* DescSet, IPipeline* Pipeline, std::vector<uint32_t> DynamicOffsets = {});

	void SetViewports(CommandBuffer* Cb, IGraphicsPipeline* Pipeline);

	void ChangeLayout(CommandBuffer* Cb, Image* Img, ImageLayout DstLayout);
}

void QueuePresent(uint32_t ImageIndex, Semaphore* ImageReadyToPresent);

uint32_t AcquireNextImage(Semaphore* ImageReadyToDraw);


