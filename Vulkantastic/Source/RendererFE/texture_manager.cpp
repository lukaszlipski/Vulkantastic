#include "texture_manager.h"
#include "dds_image.h"
#include "../Utilities/assert.h"

bool TextureManager::Shutdown()
{
	for (auto& It : mSamplersList) { It.second.reset();	}

	for (auto& It : mImageViewsList) { It.second.reset(); }

	for (auto& It : mImagesList) { It.second.reset(); }

	return true;
}

Image* TextureManager::GetImage(const std::string& Name, const ImageProperties& Properties)
{
	const ImageKey Key = std::make_tuple(Name, Properties.Format, Properties.Type, Properties.GenerateMipMaps);

	auto It = mImagesList.find(Key);

	if (It != mImagesList.end())
	{
		return It->second.get();
	}

	const uint32_t GraphicsQueueIndex = VulkanCore::Get().GetDevice()->GetQueuesIndicies().GraphicsIndex;

	const std::string FilePath = "Textures/" + Name + ".dds";

	DDSImage DDSFile(FilePath.c_str());
	void* Pixels = reinterpret_cast<void*>(DDSFile.GetData());

	const int32_t ImageSize = DDSFile.GetSize();

	if (!Pixels) { return nullptr; }

	ImageSettings Settings = {};
	Settings.Depth = 1;
	Settings.Height = DDSFile.GetHeight();
	Settings.Width = DDSFile.GetWidth();
	Settings.Format = GetFormatByDDSFormat(DDSFile.GetFormat());
	Settings.Type = GetTypeByDDSType(DDSFile.GetType());
	Settings.Mipmaps = Properties.GenerateMipMaps;

	std::vector<uint32_t> Queues = { GraphicsQueueIndex };
	auto ImageBuffer = std::make_unique<Image>(Queues, ImageUsage::SAMPLED | ImageUsage::TRANSFER_DST | ImageUsage::TRANSFER_SRC, true, Settings, Pixels);
	ImageBuffer->ChangeLayout(ImageLayout::SHADER_READ);

	mImagesList[Key] = std::move(ImageBuffer);

	return mImagesList[Key].get();

}

ImageView* TextureManager::GetImageView(Image* Img)
{
	auto It = mImageViewsList.find(Img);

	if (It != mImageViewsList.end())
	{
		return It->second.get();
	}

	ImageViewSettings ViewSettings = {};
	ViewSettings.MipMapLevelCount = Img->GetMipMapsCount();
	ViewSettings.Format = Img->GetFormat();

	auto View = std::make_unique<ImageView>(Img, ViewSettings);

	mImageViewsList[Img] = std::move(View);

	return mImageViewsList[Img].get();
}

ImageView* TextureManager::GetImageView(const std::string& Name, const ImageProperties& Properties)
{
	auto* Img = GetImage(Name, Properties);
	return GetImageView(Img);
}

Sampler* TextureManager::GetSampler(const SamplerSettings& Settings)
{
	const SamplerKey Key = std::make_tuple(Settings.WrapX, Settings.WrapY, Settings.WrapZ, Settings.MinFilter, Settings.MagFilter, Settings.MipMapFilter, Settings.MaxAnisotropy);

	auto It = mSamplersList.find(Key);
	if (It != mSamplersList.end())
	{
		return It->second.get();
	}

	auto NewSampler = std::make_unique<Sampler>(Settings);

	mSamplersList[Key] = std::move(NewSampler);

	return mSamplersList[Key].get();
}

ImageFormat TextureManager::GetFormatByDDSFormat(DDSFormat Format) const
{
	switch (Format)
	{
	case DDSFormat::R8G8B8A8: return ImageFormat::R8G8B8A8;
	case DDSFormat::R8G8B8A8_SRGB: return ImageFormat::R8G8B8A8_SRGB;
	case DDSFormat::BC1: return ImageFormat::BC1;
	case DDSFormat::BC1_SRGB: return ImageFormat::BC1_SRGB;
	}

	Assert(false); // Unsupported format

	return ImageFormat::R8G8B8A8;
}

ImageType TextureManager::GetTypeByDDSType(DDSType Type) const
{
	switch (Type)
	{
	case DDSType::ONEDIM: return ImageType::ONEDIM;
	case DDSType::TWODIM: return ImageType::TWODIM;
	case DDSType::TREEDIM: return ImageType::TREEDIM;
	}

	Assert(false); // Unsupported format

	return ImageType::TWODIM;
}
