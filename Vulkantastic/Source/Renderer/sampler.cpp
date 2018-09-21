#include "sampler.h"
#include "core.h"
#include "device.h"


Sampler::Sampler(SamplerSettings Settings)
	: mSettings(Settings)
{
	const auto Device = VulkanCore::Get().GetDevice()->GetDevice();

	VkSamplerCreateInfo SamplerInfo = {};
	SamplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	SamplerInfo.addressModeU = static_cast<VkSamplerAddressMode>(mSettings.WrapX);
	SamplerInfo.addressModeV = static_cast<VkSamplerAddressMode>(mSettings.WrapY);
	SamplerInfo.addressModeW = static_cast<VkSamplerAddressMode>(mSettings.WrapZ);
	SamplerInfo.anisotropyEnable = mSettings.MaxAnisotropy > 0 ? VK_TRUE : VK_FALSE;
	SamplerInfo.maxAnisotropy = mSettings.MaxAnisotropy;
	SamplerInfo.compareEnable = VK_FALSE;
	SamplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	SamplerInfo.magFilter = static_cast<VkFilter>(mSettings.MagFilter);
	SamplerInfo.minFilter = static_cast<VkFilter>(mSettings.MinFilter);
	SamplerInfo.maxLod = 1;
	SamplerInfo.minLod = 0;
	SamplerInfo.unnormalizedCoordinates = VK_FALSE;
	SamplerInfo.mipLodBias = .0f;
	SamplerInfo.mipmapMode = static_cast<VkSamplerMipmapMode>(mSettings.MipMapFilter);

	vkCreateSampler(Device, &SamplerInfo, nullptr, &mSampler);

}

Sampler::~Sampler()
{
	const auto Device = VulkanCore::Get().GetDevice()->GetDevice();

	vkDestroySampler(Device, mSampler, nullptr);
}
