#pragma once
#include "uniform_raw_data.h"
#include "shader_reflection.h"
#include "pipeline_manager.h"

class ShaderParameters
{
public:
	ShaderParameters(PipelineManager::KeyType Key, const std::vector<Uniform>& Uniforms, const std::map<ShaderType, std::vector<Uniform>>& PushConstants);

	ShaderParameters(const ShaderParameters& Rhs) = default;
	ShaderParameters& operator=(const ShaderParameters& Rhs) = default;

	ShaderParameters(ShaderParameters&& Rhs) = default;
	ShaderParameters& operator=(ShaderParameters&& Rhs) = default;

	UniformRawData* GetPushConstantBuffer(ShaderType Type);
	UniformRawData* GetUniformBufferByBinding(uint32_t Binding);

	inline PipelineManager::KeyType GetPipelineKey() const { return mPipelineKey; }

private:
	std::vector<UniformRawData> mUniformData;
	std::map<ShaderType, std::vector<UniformRawData>> mPushConstantData;

	PipelineManager::KeyType mPipelineKey; // PipelineManager::KeyType 
};

using upShaderParameters = std::unique_ptr<ShaderParameters>;