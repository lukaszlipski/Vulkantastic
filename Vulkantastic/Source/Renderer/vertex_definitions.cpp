#include "vertex_definitions.h"
namespace VertexDefinition {
BEGIN_VERTEX_FORMAT(Simple, false)
VERTEX_MEMBER(Simple, Vector2D, Position)
VERTEX_MEMBER(Simple, Vector3D, Color)
VERTEX_MEMBER(Simple, Vector2D, TexCoord)
END_VERTEX_FORMAT(Simple)
BEGIN_VERTEX_FORMAT(SimpleInstanced, true)
VERTEX_MEMBER(SimpleInstanced, Vector2D, Offset)
END_VERTEX_FORMAT(SimpleInstanced)
}
