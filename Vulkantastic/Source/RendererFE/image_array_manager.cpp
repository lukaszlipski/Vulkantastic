#include "image_array_manager.h"
#include "texture_manager.h"

ImageArrayManager::ImageArrayManager(DescriptorManager* DM, int32_t SetID)
{

	// Check if uniforms contain proper formats
	const std::vector<Uniform>& Templates = DM->GetUniforms(SetID);

	for (const Uniform& Template : Templates)
	{
		if (Template.Format == VariableType::IMAGE)
		{
			mImageArraySize = Template.Size;
			mImageArrayBinding = Template.Binding;
		}
		else if (Template.Format == VariableType::SAMPLER)
		{
			mSamplerArraySize = Template.Size;
			mSamplerArrayBinding = Template.Binding;
		}
		else
		{
			Assert(false); // Unsupported format
		}
	}

	mDescInst = DM->GetDescriptorInstance(SetID);
	
	const ImageView* View = TextureManager::Get().GetImageView("error");

	for (uint32_t i = 0; i < mImageArraySize; ++i)
	{
		mDescInst->SetImage(mImageArrayBinding, View, i);
	}

	Sampler* DefaultSampler = TextureManager::Get().GetSampler(RepeatSampler);

	for (uint32_t i = 0; i < mSamplerArraySize; ++i)
	{
		mDescInst->SetSampler(mSamplerArrayBinding, DefaultSampler, i);
	}


}

ImageArrayManager::ImageArrayManager(ImageArrayManager&& Rhs)
{
	*this = std::move(Rhs);
}

ImageArrayManager& ImageArrayManager::operator=(ImageArrayManager&& Rhs)
{
	mDescInst = std::move(Rhs.mDescInst);
	mImages = std::move(Rhs.mImages);

	mImageArraySize = Rhs.mImageArraySize;
	mImageArrayBinding = Rhs.mImageArrayBinding;

	mSamplerArraySize = Rhs.mSamplerArraySize;
	mSamplerArrayBinding = Rhs.mSamplerArrayBinding;

	return *this;
}

int32_t ImageArrayManager::SetImage(const std::string& Name)
{
	auto It = std::find(mImages.cbegin(), mImages.cend(), Name);

	if (It != mImages.cend())
	{
		return static_cast<int32_t>(std::distance(mImages.cbegin(), It));
	}
	else
	{
		mImages.push_back(Name);

		Assert(mImages.size() - 1 < mImageArraySize);

		return static_cast<int32_t>(mImages.size()) - 1;
	}
}

int32_t ImageArrayManager::SetSampler(const SamplerSettings& Settings)
{
	auto It = std::find(mSamplers.cbegin(), mSamplers.cend(), Settings);

	if (It != mSamplers.cend())
	{
		return static_cast<int32_t>(std::distance(mSamplers.cbegin(), It));
	}
	else
	{
		mSamplers.push_back(Settings);

		Assert(mSamplers.size() - 1 < mSamplerArraySize);

		return static_cast<int32_t>(mSamplers.size()) - 1;
	}
}

void ImageArrayManager::Update()
{
	for (uint32_t i = 0; i < mImages.size(); ++i)
	{
		ImageView* View = TextureManager::Get().GetImageView(mImages[i]);

		mDescInst->SetImage(mImageArrayBinding, View, i);
	}

	for (uint32_t i = 0; i < mSamplers.size(); ++i)
	{
		Sampler* Smp = TextureManager::Get().GetSampler(mSamplers[i]);

		mDescInst->SetSampler(mSamplerArrayBinding, Smp, i);
	}


	mDescInst->Update();

	mImages.clear();
}
