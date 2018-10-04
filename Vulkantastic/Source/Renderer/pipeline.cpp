#include "pipeline.h"
#include "shader.h"
#include "pipeline_creation.h"
#include "../Utilities/assert.h"

PipelineShaders::PipelineShaders(std::initializer_list<Shader*> Shaders)
{
	for (auto& Shader : Shaders)
	{
		switch (Shader->GetType())
		{
		case ShaderType::VERTEX:
		{
			mVertex = Shader;
			mShaders.push_back(mVertex);
			break;
		}
		case ShaderType::FRAGMENT:
		{
			mFragment = Shader;
			mShaders.push_back(mFragment);
			break;
		}
		default:
		{
			Assert(false); // Unsupported shader type
		}
		}
	}
}
