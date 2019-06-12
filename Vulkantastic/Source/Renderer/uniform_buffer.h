#pragma once
#include "shader_reflection.h"
#include <memory>
#include "../Utilities/assert.h"

class UniformBuffer
{
public:
	UniformBuffer(const Uniform& UniformData, const std::vector<uint32_t>& QueueIndicies, int32_t MaxSize = 1);
	~UniformBuffer();

	UniformBuffer(const UniformBuffer& Rhs) = delete;
	UniformBuffer& operator=(const UniformBuffer& Rhs) = delete;

	UniformBuffer(UniformBuffer&& Rhs) noexcept = delete;
	UniformBuffer& operator=(UniformBuffer&& Rhs) noexcept = delete;

	void Update();
	class Buffer* GetBuffer() const;
	std::string GetName() const;

	inline int32_t GetAlignmentSize() const { return mAlignmentSize; }

	bool Set(const class UniformRawData* UniformData, int32_t Index = 0);

private:
	Uniform mUniformDataType;
	std::unique_ptr<class Buffer> mBuffer = nullptr;
	uint8_t* mCPUData = nullptr;
	int32_t mAllocationSize = 0;
	int32_t mMaxSize = 0;
	int32_t mAlignmentSize = 0;
};

using upUniformBuffer = std::unique_ptr<UniformBuffer>;
using upUniformBufferList = std::vector<upUniformBuffer>;