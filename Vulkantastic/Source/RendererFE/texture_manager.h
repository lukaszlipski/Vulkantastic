#pragma once
#include <map>
#include "../Renderer/image.h"
#include "../Renderer/image_view.h"
#include "../Renderer/sampler.h"
#include "dds_image.h"

struct ImageProperties
{
	ImageFormat Format = ImageFormat::R8G8B8A8_SRGB;
	ImageType Type = ImageType::TWODIM;
	bool GenerateMipMaps = false;
};

struct ImageViewProperties
{
	ImageFormat Format = ImageFormat::R8G8B8A8_SRGB;
	ImageType Type = ImageType::TWODIM;
	bool GenerateMipMaps = true;
};

class TextureManager
{
public:
	static TextureManager& Get()
	{
		static TextureManager* instance = new TextureManager();
		return *instance;
	}

	bool Startup() { return true; }
	bool Shutdown();

	Image* GetImage(const std::string& Name, const ImageProperties& Properties = {});

	ImageView* GetImageView(Image* Img);
	ImageView* GetImageView(const std::string& Name, const ImageProperties& Properties = {});

	Sampler* GetSampler(const SamplerSettings& Settings = {});

private:
	TextureManager() = default;
	
	ImageFormat GetFormatByDDSFormat(DDSFormat Format) const;
	ImageType GetTypeByDDSType(DDSType Type) const;

	using ImageKey = std::tuple<std::string, ImageFormat, ImageType, bool>;
	using ImagesList = std::map<ImageKey, std::unique_ptr<Image>>;
	using ImageViewsList = std::map<Image*, std::unique_ptr<ImageView>>;
	using SamplerKey = std::tuple < AdressMode, AdressMode, AdressMode, FilterMode, FilterMode, FilterMode, float>;
	using SamplersList = std::map<SamplerKey, std::unique_ptr<Sampler>>;

	ImagesList mImagesList;
	ImageViewsList mImageViewsList;
	SamplersList mSamplersList;

};
