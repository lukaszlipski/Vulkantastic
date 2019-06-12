#pragma once
#include "shader_reflection.h"


class UniformRawData
{
public:
	UniformRawData(const Uniform& Data);

	UniformRawData(const UniformRawData& Rhs) = default;
	UniformRawData& operator=(const UniformRawData& Rhs) = default;

	UniformRawData(UniformRawData&& Rhs) = default;
	UniformRawData& operator=(UniformRawData&& Rhs) = default;

	template<typename T>
	bool Set(const std::string& Name, T Param);

	inline const uint8_t* GetBuffer() const { return mData.data(); }
	inline int32_t GetSize() const { return static_cast<int32_t>(mData.size()); }
	inline std::string GetName() const { return mUniformData.Name; }
	inline uint32_t GetBinding() const { return mUniformData.Binding; }

	int32_t GetOffset() const;
	Uniform GetType() const { return mUniformData; }

private:
	Uniform mUniformData;
	std::vector<uint8_t> mData;

};



template<typename T>
bool UniformRawData::Set(const std::string& Name, T Param)
{
	for (const auto& Member : mUniformData.Members)
	{
		if (Member.Name == Name)
		{
			const auto Offset = Member.Offset - GetOffset();
			const auto Size = ShaderReflection::GetSizeForFormat(Member.Format);

			Assert(Size == sizeof(Param));

			memcpy(reinterpret_cast<void*>(&mData[Offset]), &Param, Size);

			return true;
		}
	}
	return false;
}
