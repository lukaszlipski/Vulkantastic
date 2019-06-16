#pragma once
#include <string>
#include "../Renderer/descriptor_manager.h"
#include "../Renderer/pipeline_manager.h"
#include "../Renderer/vertex_definitions.h"
#include "../Utilities/assert.h"
#include "deferred_renderer.h"
#include "texture_manager.h"
#include "../Renderer/shader_parameters.h"

template<typename ...T>
class SurfaceMaterial
{
	friend DeferredRenderer;

public:

	SurfaceMaterial(const SurfaceMaterial& Rhs);
	SurfaceMaterial& operator=(const SurfaceMaterial& Rhs);

	SurfaceMaterial(SurfaceMaterial&& Rhs) noexcept;
	SurfaceMaterial& operator=(SurfaceMaterial&& Rhs) noexcept;

	SurfaceMaterial(const std::string& VertexShader, const std::string& FragmentShader);

	SurfaceMaterial& SetMVP(const glm::mat4x4& MVP);
	SurfaceMaterial& SetMV(const glm::mat4x4& MV);
	SurfaceMaterial& SetCustomColor(const glm::vec3& CustomColor); 
	SurfaceMaterial& SetAlbedoTexture(const std::string& Name);

	inline ShaderParameters* GetShaderParameters() const { return mShaderParams.get(); }

	std::map<std::string, std::string> GetUsedImages() const { return mImages; }
	std::map<std::string, SamplerSettings> GetUsedSamplers() const { return mSamplers; }

	IGraphicsPipeline* GetPipeline() const;

	void Update();

private:

	upShaderParameters mShaderParams;

	std::map<std::string, std::string> mImages;  // PushConstantName -> ImageName
	std::map<std::string, SamplerSettings> mSamplers;  // PushConstantName -> SamplerSettings

	std::string mVertexShader;
	std::string mFragmentShader;

};

template<typename ...T>
SurfaceMaterial<T...>::SurfaceMaterial(const SurfaceMaterial& Rhs)
{
	*this = Rhs;
}

using StaticSurfaceMaterial = SurfaceMaterial<VertexDefinition::StaticMesh>;


template<typename ...T>
SurfaceMaterial<T...>& SurfaceMaterial<T...>::operator=(const SurfaceMaterial& Rhs)
{
	mVertexShader = Rhs.mVertexShader;
	mFragmentShader = Rhs.mFragmentShader;

	ShaderParameters* Ptr = Rhs.mShaderParams.get();
	
	if (Ptr != nullptr)
	{
		mShaderParams = std::make_unique<ShaderParameters>(*Ptr);
	}

	mImages = Rhs.mImages;

	return *this;
}

template<typename ...T>
SurfaceMaterial<T...>::SurfaceMaterial(SurfaceMaterial<T...>&& Rhs) noexcept
{
	*this = std::move(Rhs);
}

template<typename ...T>
SurfaceMaterial<T...>& SurfaceMaterial<T...>::operator=(SurfaceMaterial<T...>&& Rhs) noexcept
{
	mVertexShader = std::move(Rhs.mVertexShader);
	mFragmentShader = std::move(Rhs.mFragmentShader);

	mShaderParams = std::move(Rhs.mShaderParams);

	mImages = std::move(Rhs.mImages);

	return *this;
}

template<typename ...T>
void SurfaceMaterial<T...>::Update()
{
	//mDescriptorInstance->Update();
}

template<typename ...T>
IGraphicsPipeline* SurfaceMaterial<T...>::GetPipeline() const
{
	return PipelineManager::Get().GetPipelineByKey(mShaderParams->GetPipelineKey());
}

template<typename ...T>
SurfaceMaterial<T...>::SurfaceMaterial(const std::string& VertexShaderName, const std::string& FragmentShaderName)
	: mVertexShader(VertexShaderName), mFragmentShader(FragmentShaderName)
{
	Shader* VertexShader = ShaderManager::Get().Find(VertexShaderName);
	Shader* FragmentShader = ShaderManager::Get().Find(FragmentShaderName);

	Assert(VertexShader && FragmentShader);

	PipelineShaders Shaders{ VertexShader, FragmentShader };

	RenderPass* Rp = DeferredRenderer::Get().GetBasePassRenderPass();

	mShaderParams = PipelineManager::Get().GetShaderParametersInstance<T...>(*Rp, Shaders, 1); // Set 1 should contain uniform buffer

	// Define default images used by material
	mImages["AlbedoIdx"] = "test";

	// Define default samplers used by material
	mSamplers["WrapIdx"] = WrapSampler;
	mSamplers["RepeatIdx"] = RepeatSampler;

}

template<typename ...T>
SurfaceMaterial<T...>& SurfaceMaterial<T...>::SetMVP(const glm::mat4x4& MVP)
{

	UniformRawData* RawData = mShaderParams->GetUniformBufferByBinding(0); // Vertex shader's uniform buffer
	RawData->Set("MVP2", MVP);

	return *this;
}


template<typename ...T>
SurfaceMaterial<T...>& SurfaceMaterial<T...>::SetMV(const glm::mat4x4& MV)
{

	UniformRawData* RawData = mShaderParams->GetUniformBufferByBinding(0); // Vertex shader's uniform buffer
	RawData->Set("MV2", MV);

	return *this;
}


template<typename ...T>
SurfaceMaterial<T...>& SurfaceMaterial<T...>::SetCustomColor(const glm::vec3& CustomColor)
{
	//auto PCFragPtr = mShaderParams->GetPushConstantBuffer(ShaderType::FRAGMENT);
	//PCFragPtr->Set("CustomColor", CustomColor);

	UniformRawData* RawData = mShaderParams->GetUniformBufferByBinding(1); // Fragment shader's uniform buffer
	RawData->Set("CustomColor2", CustomColor);

	return *this;
}

template<typename ...T>
SurfaceMaterial<T...>& SurfaceMaterial<T...>::SetAlbedoTexture(const std::string& Name)
{
	mImages["AlbedoIdx"] = Name;
	
	return *this;
}


