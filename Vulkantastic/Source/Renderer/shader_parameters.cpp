#include "shader_parameters.h"
#include "../Utilities/assert.h"
#include "uniform_raw_data.h"

ShaderParameters::ShaderParameters(PipelineManager::KeyType Key, const std::vector<Uniform>& Uniforms, const std::map<ShaderType, std::vector<Uniform>>& PushConstants)
	: mPipelineKey(Key)
{

	for (const Uniform& Template : Uniforms)
	{
		if (Template.Format == VariableType::STRUCTURE)
		{
			mUniformData.emplace_back(Template);
		}
		else if (Template.Format == VariableType::SAMPLER)
		{

		}
		else
		{
			Assert(false); // Unsupported format
		}
	}

	for (const std::pair<ShaderType, std::vector<Uniform>>& Constants : PushConstants)
	{
		const ShaderType& Type = Constants.first;
		const std::vector<Uniform>& Uniforms = Constants.second;

		for (const Uniform& Template : Uniforms)
		{
			if (Template.Format == VariableType::STRUCTURE)
			{
				mPushConstantData[Type].emplace_back( std::move(UniformRawData(Template)) );
			}
			else
			{
				Assert(false); // Unsupported format
			}

		}

	}

}

UniformRawData* ShaderParameters::GetPushConstantBuffer(ShaderType Type)
{
	auto PushConstant = mPushConstantData.find(Type);
	if (PushConstant != mPushConstantData.end())
	{
		return &PushConstant->second.front();
	}
	return nullptr;
}

UniformRawData* ShaderParameters::GetUniformBufferByBinding(uint32_t Binding)
{
	auto Data = std::find_if(begin(mUniformData), end(mUniformData), [Binding](const UniformRawData& Data) {
		return Data.GetBinding() == Binding;
	});

	return Data != mUniformData.end() ? &(*Data) : nullptr;
}
