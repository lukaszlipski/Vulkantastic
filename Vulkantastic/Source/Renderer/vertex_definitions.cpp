#include "vertex_definitions.h"
namespace VertexDefinition {
BEGIN_VERTEX_FORMAT(Simple, false)
VERTEX_MEMBER(Simple, glm::vec2, Position)
VERTEX_MEMBER(Simple, glm::vec3, Color)
VERTEX_MEMBER(Simple, glm::vec2, TexCoord)
END_VERTEX_FORMAT(Simple)
BEGIN_VERTEX_FORMAT(SimpleInstanced, true)
VERTEX_MEMBER(SimpleInstanced, glm::vec2, Offset)
END_VERTEX_FORMAT(SimpleInstanced)
}
