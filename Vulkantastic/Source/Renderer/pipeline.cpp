#include "pipeline.h"
#include "shader.h"
#include "pipeline_creation.h"

PipelineShaders::PipelineShaders(std::initializer_list<Shader*> Shaders)
{
	for (auto& Shader : Shaders)
	{
		switch (Shader->GetType())
		{
		case ShaderType::VERTEX:
		{
			mVertex = Shader;
			break;
		}
		case ShaderType::FRAGMENT:
		{
			mFragment = Shader;
			break;
		}
		}
	}
}
