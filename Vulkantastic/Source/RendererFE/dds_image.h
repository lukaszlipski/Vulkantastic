#pragma once

enum class DDSFormat
{
	R8G8B8A8,
	R8G8B8A8_SRGB,
	BC1,
	BC1_SRGB
};

enum class DDSType
{
	ONEDIM,
	TWODIM,
	TREEDIM
};

class DDSImage
{
public:

	DDSImage(const std::string& Name);
	~DDSImage();

	DDSImage(const DDSImage& Rhs);
	DDSImage& operator=(const DDSImage& Rhs);

	DDSImage(DDSImage&& Rhs);
	DDSImage& operator=(DDSImage&& Rhs);

	inline bool HasMipMaps() const { return mMipMapsPixelsData.size() > 0; }
	inline bool IsValid() const { return mPixelsData.size() > 0; }
	inline int32_t GetSize() const { return static_cast<int32_t>(mPixelsData.size()); }
	inline uint8_t* GetData() { return IsValid() ? &mPixelsData[0] : nullptr; }
	inline uint8_t* GetMipMapData() { return IsValid() ? &mMipMapsPixelsData[0] : nullptr; }
	inline int32_t GetWidth() const { return mWidth; }
	inline int32_t GetHeight() const { return mHeight; }
	inline DDSFormat GetFormat() const { return mFormat; }
	inline DDSType GetType() const { return mType; }

private:
	std::vector<uint8_t> mPixelsData;
	std::vector<uint8_t> mMipMapsPixelsData;
	std::string mName;
	int32_t mWidth = -1;
	int32_t mHeight = -1;
	DDSFormat mFormat = DDSFormat::R8G8B8A8;
	DDSType mType = DDSType::TWODIM;

};
