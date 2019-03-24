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

	struct StaticMesh
	{
		DECLARE_VERTEX_FORMAT()
		glm::vec3 Position;
		glm::vec2 TexCoord;
		glm::vec3 Normal;
		glm::vec3 Tangent;
	};

	struct SimpleInstanced
	{
		DECLARE_VERTEX_FORMAT_INST()
		glm::vec3 Offset;
	};

	struct SimpleScreen
	{
		DECLARE_VERTEX_FORMAT()
		glm::vec2 Position;
		glm::vec2 TexCoord;
	};

}
