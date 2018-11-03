#pragma once
#include "vertex_definitions_inc.h"
#include "glm/glm.hpp"

namespace VertexDefinition
{
	struct Simple
	{
		DECLARE_VERTEX_FORMAT()
		glm::vec2 Position;
		glm::vec3 Color;
		glm::vec2 TexCoord;
	};

	struct SimpleInstanced
	{
		DECLARE_VERTEX_FORMAT_INST()
		glm::vec2 Offset;
	};

}
