#pragma once
#include "shader_reflection.h"

class PushConstantBuffer
{
public:
	PushConstantBuffer(const Uniform& PushConstantData);
	~PushConstantBuffer();

	inline uint8_t* GetBuffer() const { return mCPUData; }
	inline int32_t GetSize() const { return mAllocationSize; }

	int32_t GetOffset() const;

	template<typename T>
	bool Set(const std::string& Name, T Param);

private:
	Uniform mPushConstantData;
	uint8_t* mCPUData = nullptr;
	int32_t mAllocationSize = 0;

};

template<typename T>
bool PushConstantBuffer::Set(const std::string& Name, T Param)
{
	for (const auto& Member : mPushConstantData.Members)
	{
		if (Member.Name == Name)
		{
			const auto Offset = Member.Offset - GetOffset();
			const auto Size = ShaderReflection::GetSizeForFormat(Member.Format);

			Assert(Size == sizeof(Param));

			memcpy(reinterpret_cast<void*>(&mCPUData[Offset]), &Param, Size);

			return true;
		}
	}
	return false;
}
