#include "pipeline_manager.h"
#include <numeric>

bool PipelineManager::Shutdown()
{
	for (auto& Pipeline : mPipelines)
	{
		delete Pipeline.second;
	}
	return true;
}

PipelineManager::KeyType PipelineManager::HashShaders(const std::vector<Shader*>& Shaders) const
{
	// For now I take combined names of shaders as a key
	const std::string KeyResult = std::accumulate(cbegin(Shaders), cend(Shaders), std::string{}, [](std::string CurrentResult, const Shader* CurrentShader) {
		return CurrentResult += CurrentShader->GetName();
	});

	return KeyResult;
}

IGraphicsPipeline* PipelineManager::GetPipelineByKey(KeyType Key)
{
	auto It = mPipelines.find(Key);
	return It != mPipelines.end() ? It->second : nullptr;
}

