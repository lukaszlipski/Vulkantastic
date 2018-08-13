#pragma once
#include "vertex_definitions_inc.h"
#include "../Math/vector2d.h"
#include "../Math/vector3d.h"

namespace VertexDefinition
{
	struct Simple
	{
		DECLARE_VERTEX_FORMAT()
		Vector2D Position;
		Vector3D Color;
	};

}
