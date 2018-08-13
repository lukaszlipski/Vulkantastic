#pragma once
#include <cstdint>
#include <vector>

#define DECLARE_VERTEX_FORMAT() static VertexFormatDeclaration VertexFormatInfo;
#define BEGIN_VERTEX_FORMAT(name) VertexFormatDeclaration name::VertexFormatInfo = { #name, {
#define VERTEX_MEMBER(name, type, member) {#member, #type ,sizeof(name::member),offsetof(name,member)},
#define END_VERTEX_FORMAT(name) }, sizeof(name)}; 

struct VertexAttribute
{
	const char* Name;
	const char* Type;
	uint32_t Size;
	uint32_t Offset;
};

struct VertexFormatDeclaration
{
	const char* Name;
	std::vector<VertexAttribute> Members;
	int32_t Size;
};
