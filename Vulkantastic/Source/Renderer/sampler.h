#pragma once
#include "vulkan/vulkan_core.h"

enum class AdressMode
{
	REPEAT = VK_SAMPLER_ADDRESS_MODE_REPEAT,
	BORDER = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER
};

enum class FilterMode
{
	LINEAR = VK_SAMPLER_MIPMAP_MODE_LINEAR,
	NEAREST = VK_SAMPLER_MIPMAP_MODE_NEAREST
};

struct SamplerSettings
{
	AdressMode WrapX = AdressMode::REPEAT;
	AdressMode WrapY = AdressMode::REPEAT;
	AdressMode WrapZ = AdressMode::REPEAT;
	FilterMode MinFilter = FilterMode::LINEAR;
	FilterMode MagFilter = FilterMode::LINEAR;
	FilterMode MipMapFilter = FilterMode::LINEAR;
	float MaxAnisotropy = 16;

	friend bool operator==(const SamplerSettings& Lhs, const SamplerSettings& Rhs)
	{
		return	Lhs.WrapX == Rhs.WrapX && Lhs.WrapY == Rhs.WrapY && Lhs.WrapZ == Rhs.WrapZ &&
				Lhs.MinFilter == Rhs.MinFilter && Lhs.MagFilter == Rhs.MagFilter && Lhs.MipMapFilter == Rhs.MipMapFilter &&
				Lhs.MaxAnisotropy == Rhs.MaxAnisotropy;
	}

};

class Sampler
{
public:
	Sampler(SamplerSettings Settings);
	~Sampler();

	inline VkSampler GetSampler() const { return mSampler; }

private:
	VkSampler mSampler = nullptr;
	SamplerSettings mSettings;
};


// Default sampler settings

constexpr SamplerSettings RepeatSampler = SamplerSettings{ };
constexpr SamplerSettings WrapSampler = SamplerSettings{ AdressMode::BORDER, AdressMode::BORDER, AdressMode::BORDER };

