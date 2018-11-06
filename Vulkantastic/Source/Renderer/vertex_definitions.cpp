#include "vertex_definitions.h"
namespace VertexDefinition {
BEGIN_VERTEX_FORMAT(Simple, false)
VERTEX_MEMBER(Simple, glm::vec2, Position)
VERTEX_MEMBER(Simple, glm::vec3, Color)
VERTEX_MEMBER(Simple, glm::vec2, TexCoord)
END_VERTEX_FORMAT(Simple)
BEGIN_VERTEX_FORMAT(StaticMesh, false)
VERTEX_MEMBER(StaticMesh, glm::vec3, Position)
VERTEX_MEMBER(StaticMesh, glm::vec2, TexCoord)
VERTEX_MEMBER(StaticMesh, glm::vec3, Normal)
VERTEX_MEMBER(StaticMesh, glm::vec3, Tangent)
END_VERTEX_FORMAT(StaticMesh)
BEGIN_VERTEX_FORMAT(SimpleInstanced, true)
VERTEX_MEMBER(SimpleInstanced, glm::vec3, Offset)
END_VERTEX_FORMAT(SimpleInstanced)
}
