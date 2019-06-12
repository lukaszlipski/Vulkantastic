#include "uniform_raw_data.h"
#include "../Utilities/assert.h"

UniformRawData::UniformRawData(const Uniform& Data)
	: mUniformData(Data)
{
	Assert(mUniformData.Format == VariableType::STRUCTURE);

	int32_t Size = ShaderReflection::GetSizeForStructure(mUniformData);

	mData.resize(Size);

}

int32_t UniformRawData::GetOffset() const
{
	Assert(mUniformData.Members.size() > 0);

	return mUniformData.Members[0].Offset;
}
