#include "uniform_buffer.h"
#include "buffer.h"
#include "../Utilities/assert.h"

UniformBuffer::UniformBuffer(const Uniform& ParametersData, const std::vector<uint32_t>& QueueIndicies)
	: mUniformData(ParametersData)
{
	Assert(mUniformData.Format == VariableType::STRUCTURE);

	mAllocationSize = ShaderReflection::GetSizeForStructure(mUniformData);

	mCPUData = new uint8_t[mAllocationSize];
	memset(mCPUData, 0, mAllocationSize);
	
	mBuffer = std::make_unique<Buffer>(QueueIndicies, BufferUsage::UNIFORM, false, mAllocationSize, mCPUData);
	
}

UniformBuffer::~UniformBuffer()
{
	delete[] mCPUData;
}

void UniformBuffer::Update()
{
	mBuffer->UploadData(mCPUData, mAllocationSize, 0);
}

class Buffer* UniformBuffer::GetBuffer() const
{
	return mBuffer.get();
}

std::string UniformBuffer::GetName() const
{
	return mUniformData.Name;
}
