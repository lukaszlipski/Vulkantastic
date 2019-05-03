#include "dds_image.h"
#include "../File/file.h"
#include "../Utilities/assert.h"
#include <dxgiformat.h>
#include <d3d10.h>
#include <ddraw.h>

struct DDS_PIXELFORMAT 
{
	int32_t dwSize;
	int32_t dwFlags;
	int32_t dwFourCC;
	int32_t dwRGBBitCount;
	int32_t dwRBitMask;
	int32_t dwGBitMask;
	int32_t dwBBitMask;
	int32_t dwABitMask;
};

struct DDS_HEADER 
{
	int32_t dwSize;
	int32_t dwFlags;
	int32_t dwHeight;
	int32_t dwWidth;
	int32_t dwPitchOrLinearSize;
	int32_t dwDepth;
	int32_t dwMipMapCount;
	int32_t dwReserved1[11];
	DDS_PIXELFORMAT ddspf;
	int32_t dwCaps;
	int32_t dwCaps2;
	int32_t dwCaps3;
	int32_t dwCaps4;
	int32_t dwReserved2;
};

struct DDS_HEADER_DXT10
{
	DXGI_FORMAT dxgiFormat;
	D3D10_RESOURCE_DIMENSION resourceDimension;
	uint32_t miscFlag;
	uint32_t arraySize;
	uint32_t miscFlags2;
};

int32_t GetRawSizeByFormat(DXGI_FORMAT Format, int32_t Width, int32_t Height)
{
	switch (Format)
	{
	case DXGI_FORMAT_R8G8B8A8_UNORM:
	case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB: 
	{
		return static_cast<int>(Width * Height * 4.0f);
	}
	case DXGI_FORMAT_BC1_UNORM:
	{
		return static_cast<int>(std::ceil(Width / 4.0f) * std::ceil(Height / 4.0f) * 16.0f);
	}
	}

	Assert(false); // Unsupported format

	return -1;
	
}

DDSFormat GetDDSFormatByDX10Format(DXGI_FORMAT Format)
{
	switch (Format)
	{
	case DXGI_FORMAT_R8G8B8A8_UNORM: return DDSFormat::R8G8B8A8;
	case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB: return DDSFormat::R8G8B8A8_SRGB;
	case DXGI_FORMAT_BC1_UNORM: return DDSFormat::BC1;
	case DXGI_FORMAT_BC1_UNORM_SRGB: return DDSFormat::R8G8B8A8_SRGB;
	}

	Assert(false); // Unsupported format

	return DDSFormat::R8G8B8A8;
}

DDSType GetDDSTypeByDX10Type(D3D10_RESOURCE_DIMENSION Type)
{
	switch (Type)
	{
	case D3D10_RESOURCE_DIMENSION_TEXTURE1D: return DDSType::ONEDIM;
	case D3D10_RESOURCE_DIMENSION_TEXTURE2D: return DDSType::TWODIM;
	case D3D10_RESOURCE_DIMENSION_TEXTURE3D: return DDSType::TREEDIM;
	}

	Assert(false); // Unsupported format

	return DDSType::TWODIM;
}

DDSImage::DDSImage(const std::string& Name)
	: mName(Name)
{

	const int32_t FileSize = static_cast<int32_t>(File::Get().Size(mName));
	const int32_t HeaderSize = sizeof(DDS_HEADER);
	const int32_t DX10HeaderSize = sizeof(DDS_HEADER_DXT10);
	
	DDS_HEADER Header;
	DDS_HEADER_DXT10 DX10Header;

	FileGuard SourceHandle(File::Get().OpenRead(mName));

	int32_t MagicNumber;
	SourceHandle->Read(reinterpret_cast<uint8_t*>(&MagicNumber), sizeof(int32_t));

	Assert(MagicNumber == 0x20534444);
	
	SourceHandle->Read(reinterpret_cast<uint8_t*>(&Header), HeaderSize);
	SourceHandle->Read(reinterpret_cast<uint8_t*>(&DX10Header), DX10HeaderSize);
	
	const bool HasMipMaps = Header.dwMipMapCount > 0;

	mWidth = Header.dwWidth;
	mHeight = Header.dwHeight;

	mFormat = GetDDSFormatByDX10Format(DX10Header.dxgiFormat);
	mType = GetDDSTypeByDX10Type(DX10Header.resourceDimension);

	if (DX10Header.resourceDimension == D3D10_RESOURCE_DIMENSION_TEXTURE2D)
	{
		int32_t RawSize = GetRawSizeByFormat(DX10Header.dxgiFormat, Header.dwWidth, Header.dwHeight);
		
		mPixelsData.resize(RawSize);

		SourceHandle->Read(reinterpret_cast<uint8_t*>(mPixelsData.data()), RawSize);
		
	}
	else
	{
		Assert(false); // For now there is only support for 2D textures
	}

}

DDSImage::~DDSImage()
{

}
