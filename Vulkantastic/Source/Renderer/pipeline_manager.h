#pragma once
#include "render_pass.h"
#include "pipeline.h"
#include <map>
#include "descriptor_manager.h"

class PipelineManager
{
public:
	static PipelineManager& Get()
	{
		static PipelineManager* instance = new PipelineManager();
		return *instance;
	}

	PipelineManager() = default;

	PipelineManager(const PipelineManager& Rhs) = delete;
	PipelineManager& operator=(const PipelineManager& Rhs) = delete;

	PipelineManager(PipelineManager&& Rhs) = delete;
	PipelineManager& operator=(PipelineManager&& Rhs) = delete;

	bool Startup() { return true; }
	bool Shutdown();

	using KeyType = std::string;

	KeyType HashShaders(const std::vector<Shader*>& Shaders) const;

	template<typename ...T>
	std::unique_ptr<class DescriptorInst> GetDescriptorInstance(const RenderPass& GraphicsRenderPass, PipelineShaders Shaders);

	template<typename ...T>
	std::unique_ptr<class ShaderParameters> GetShaderParametersInstance(const RenderPass& GraphicsRenderPass, PipelineShaders Shaders);

	IGraphicsPipeline* GetPipelineByKey(KeyType Key);

private:
	std::map<KeyType, IGraphicsPipeline*> mPipelines;

};

template<typename ...T>
std::unique_ptr<DescriptorInst> PipelineManager::GetDescriptorInstance(const RenderPass& GraphicsRenderPass, PipelineShaders Shaders)
{
	using CurrentPipelineType = GraphicsPipeline<T...>;

	const KeyType KeyResult = HashShaders(Shaders.GetShaders());
	auto It = mPipelines.find(KeyResult);
	if (It != end(mPipelines))
	{
		CurrentPipelineType* Pipeline = static_cast<CurrentPipelineType*>(It->second);
		return Pipeline->GetDescriptorManager()->GetDescriptorInstance();
	}

	CurrentPipelineType* NewEntry = new CurrentPipelineType(GraphicsRenderPass, Shaders);
	mPipelines[KeyResult] = NewEntry;

	return NewEntry->GetDescriptorManager()->GetDescriptorInstance();
}

template<typename ...T>
std::unique_ptr<ShaderParameters> PipelineManager::GetShaderParametersInstance(const RenderPass& GraphicsRenderPass, PipelineShaders Shaders)
{
	using CurrentPipelineType = GraphicsPipeline<T...>;

	const KeyType KeyResult = HashShaders(Shaders.GetShaders());
	auto It = mPipelines.find(KeyResult);
	if (It != end(mPipelines))
	{
		CurrentPipelineType* Pipeline = static_cast<CurrentPipelineType*>(It->second);
		return Pipeline->GetDescriptorManager()->GetShaderParametersInstance();
	}

	CurrentPipelineType* NewEntry = new CurrentPipelineType(GraphicsRenderPass, Shaders);
	mPipelines[KeyResult] = NewEntry;

	return NewEntry->GetDescriptorManager()->GetShaderParametersInstance();
}
