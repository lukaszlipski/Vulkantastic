#pragma once
#include <map>
#include "../Renderer/vertex_definitions.h"
#include <memory>
#include "surface_material.h"


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

	int32_t GetVertexBufferCount() const { return static_cast<int32_t>(mVertexBuffers.size()); }

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

class StaticMeshHandle
{
public:
	StaticMeshHandle(const std::string& Name);

	StaticMeshHandle(const StaticMeshHandle& Name) = default;
	StaticMeshHandle(StaticMeshHandle&& Name) = default;

	StaticMeshHandle& SetMaterial(int32_t Id, const StaticSurfaceMaterial& Material);
	StaticSurfaceMaterial* GetMaterial(int32_t Id);

	inline int32_t GetMaterialsCount() const { return static_cast<int32_t>(mMaterials.size());}
	inline const StaticMesh* GetStaticMesh() const { return mStaticMesh; }
	inline std::string GetStaticMeshName() const { return mName; }

private:
	using MaterialList = std::vector<StaticSurfaceMaterial>;

	StaticMesh* mStaticMesh = nullptr;
	std::string mName;

	MaterialList mMaterials;
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
