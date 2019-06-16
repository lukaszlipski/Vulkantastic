#pragma once
#include "../Renderer/descriptor_manager.h"

class ImageArrayManager
{
public:
	ImageArrayManager(DescriptorManager* DM, int32_t SetID = 0);
	~ImageArrayManager() = default;

	ImageArrayManager(const ImageArrayManager& Rhs) = delete;
	ImageArrayManager& operator=(const ImageArrayManager& Rhs) = delete;

	ImageArrayManager(ImageArrayManager&& Rhs);
	ImageArrayManager& operator=(ImageArrayManager&& Rhs);

	DescriptorInst* GetDescInst() { return mDescInst.get(); }

	int32_t SetImage(const std::string& Name);
	int32_t SetSampler(const SamplerSettings& Settings);

	void Update();

private:

	upDescriptorInst mDescInst;

	std::vector<std::string> mImages;
	uint32_t mImageArraySize;
	uint32_t mImageArrayBinding;

	std::vector<SamplerSettings> mSamplers;
	uint32_t mSamplerArraySize;
	uint32_t mSamplerArrayBinding;

};

using upImageArrayManager = std::unique_ptr<ImageArrayManager>;
