#pragma once
#include <map>
#include "../Renderer/vertex_definitions.h"
#include <memory>

class Buffer;

class StaticMesh
{
public:
	StaticMesh(const std::string& Name);
	~StaticMesh();

	StaticMesh(const StaticMesh& Rhs) = delete;
	StaticMesh& operator=(const StaticMesh& Rhs) = delete;

	StaticMesh(StaticMesh&& Rhs) = delete;
	StaticMesh& operator=(StaticMesh&& Rhs) = delete;

	Buffer* GetVertexBuffer(int32_t Index = 0) const;
	Buffer* GetIndexBuffer(int32_t Index = 0) const;
	uint32_t GetIndiciesSize(int32_t Index = 0) const;

private:
	std::string mName;

	using VerticiesList = std::vector<VertexDefinition::StaticMesh>;
	using IndiciesList = std::vector<uint32_t>;
	using BufferList = std::unique_ptr<Buffer>;

	std::vector<VerticiesList> mVertices;
	std::vector<IndiciesList> mIndicies;

	std::vector<BufferList> mVertexBuffers;
	std::vector<BufferList> mIndexBuffers;

};

class StaticMeshManager
{
public:
	static StaticMeshManager& Get()
	{
		static StaticMeshManager* instance = new StaticMeshManager();
		return *instance;
	}

	bool Startup();
	bool Shutdown();

	StaticMesh* Find(const std::string& Name);

private:
	StaticMeshManager() = default;

	std::map<std::string, StaticMesh*> mStaticMeshList;

};
