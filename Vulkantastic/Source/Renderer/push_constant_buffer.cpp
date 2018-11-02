#include "push_constant_buffer.h"
#include "../Utilities/assert.h"


PushConstantBuffer::PushConstantBuffer(const Uniform& PushConstantData)
	: mPushConstantData(PushConstantData)
{
	Assert(mPushConstantData.Format == VariableType::STRUCTURE);

	mAllocationSize = ShaderReflection::GetSizeForStructure(mPushConstantData);

	mCPUData = new uint8_t[mAllocationSize];
	memset(mCPUData, 0, mAllocationSize);

}

PushConstantBuffer::~PushConstantBuffer()
{
	delete[] mCPUData;
}

int32_t PushConstantBuffer::GetOffset() const
{
	Assert(mPushConstantData.Members.size() > 0);

	return mPushConstantData.Members[0].Offset;
}
