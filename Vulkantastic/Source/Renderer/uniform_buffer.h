#pragma once
#include "shader_reflection.h"
#include <memory>

class UniformBuffer
{
public:
	UniformBuffer(const Uniform& UniformData, const std::vector<uint32_t>& QueueIndicies);
	~UniformBuffer();

	UniformBuffer(const UniformBuffer& Rhs) = delete;
	UniformBuffer& operator=(const UniformBuffer& Rhs) = delete;

	UniformBuffer(UniformBuffer&& Rhs) noexcept = delete;
	UniformBuffer& operator=(UniformBuffer&& Rhs) noexcept = delete;

	void Update();
	class Buffer* GetBuffer() const;
	std::string GetName() const;

	template<typename T>
	bool Set(const std::string& Name, T Param);

private:
	Uniform mUniformData;
	std::unique_ptr<class Buffer> mBuffer = nullptr;
	uint8_t* mCPUData = nullptr;
	int32_t mAllocationSize = 0;
};

template<typename T>
bool UniformBuffer::Set(const std::string& Name, T Param)
{
	for (const auto& Member : mUniformData.Members)
	{
		if (Member.Name == Name)
		{
			const auto Offset = Member.Offset;
			const auto Size = ShaderReflection::GetSizeForFormat(Member.Format);

			Assert(Size == sizeof(Param));

			memcpy(reinterpret_cast<void*>(&mCPUData[Offset]), &Param, Size);

			return true;
		}
	}
	return false;
}
