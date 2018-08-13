#include "vertex_definitions.h"
namespace VertexDefinition {
BEGIN_VERTEX_FORMAT(Simple)
VERTEX_MEMBER(Simple, Vector2D, Position)
VERTEX_MEMBER(Simple, Vector3D, Color)
END_VERTEX_FORMAT(Simple)
}
