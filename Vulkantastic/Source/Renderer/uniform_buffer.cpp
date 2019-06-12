#include "uniform_buffer.h"
#include "buffer.h"
#include "../Utilities/assert.h"
#include "device.h"
#include "core.h"
#include "uniform_raw_data.h"

UniformBuffer::UniformBuffer(const Uniform& ParametersData, const std::vector<uint32_t>& QueueIndicies, int32_t MaxSize)
	: mUniformDataType(ParametersData), mMaxSize(MaxSize)
{
	Assert(mUniformDataType.Format == VariableType::STRUCTURE);

	auto Device = VulkanCore::Get().GetDevice();
	const VkPhysicalDeviceLimits& limits = Device->GetLimits();

	int32_t Alignment = static_cast<int32_t>(limits.minUniformBufferOffsetAlignment) - 1;

	int32_t UniformSize = ShaderReflection::GetSizeForStructure(mUniformDataType);
	mAlignmentSize = ((UniformSize) + Alignment) & ~Alignment; // Assumption that alignment is power of two
	mAllocationSize = mAlignmentSize * MaxSize;
	
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
	return mUniformDataType.Name;
}

bool UniformBuffer::Set(const UniformRawData* UniformData, int32_t Index /*= 0*/)
{
	const Uniform Type = UniformData->GetType();
	const int32_t UniformSize = ShaderReflection::GetSizeForStructure(Type);

	Assert(Type.Format == mUniformDataType.Format && Type.Size == mUniformDataType.Size && Index >= 0 && Index < mMaxSize);

	memcpy(&mCPUData[mAlignmentSize * Index], UniformData->GetBuffer(), UniformSize);

	return true;
}
