#include "static_mesh.h"
#include "../File/file.h"
#include "../Renderer/buffer.h"
#include "../Utilities/assert.h"
#include "surface_material.h"

bool StaticMeshManager::Startup()
{
	return true;
}

bool StaticMeshManager::Shutdown()
{
	for (auto& StaticMeshToDelete : mStaticMeshList)
	{
		delete StaticMeshToDelete.second;
	}

	return true;
}

StaticMesh* StaticMeshManager::Find(const std::string& Name)
{
	auto It = mStaticMeshList.find(Name);
	if (It != mStaticMeshList.end())
	{
		return mStaticMeshList[Name];
	}

	const std::string StaticMeshPath = File::Get().CurrentDirectory() + "/Meshes/" + Name + ".sm";

	if (!File::Get().Exists(StaticMeshPath)) { return nullptr; }

	auto* NewEntry = new StaticMesh(StaticMeshPath);

	mStaticMeshList[Name] = NewEntry;
	
	return NewEntry;
}

StaticMesh::StaticMesh(const std::string& Name)
	: mName(Name)
{
	int32_t FileSize = static_cast<int32_t>(File::Get().Size(mName));

	FileGuard SourceHandle(File::Get().OpenRead(mName));

	while (FileSize > SourceHandle->ReadBytes())
	{

		const uint32_t GraphicsQueueIndex = VulkanCore::Get().GetDevice()->GetQueuesIndicies().GraphicsIndex;
		std::vector<uint32_t> QueueIndicies = { GraphicsQueueIndex };

		// Vertex attributes
		int32_t VertAttribLength;
		SourceHandle->Read(reinterpret_cast<uint8_t*>(&VertAttribLength), 4);

		VerticiesList Verticies(VertAttribLength * sizeof(VertexDefinition::StaticMesh));
		SourceHandle->Read(reinterpret_cast<uint8_t*>(Verticies.data()), VertAttribLength * sizeof(VertexDefinition::StaticMesh));

		mVertexBuffers.push_back(std::make_unique<Buffer>(QueueIndicies, BufferUsage::VERTEX | BufferUsage::TRANSFER_DST, true, static_cast<uint32_t>(sizeof(VertexDefinition::StaticMesh) * Verticies.size()), Verticies.data()));

		mVertices.push_back(std::move(Verticies));

		// Indicies
		int32_t IndiciesLength;
		SourceHandle->Read(reinterpret_cast<uint8_t*>(&IndiciesLength), 4);

		IndiciesList Indicies(IndiciesLength * sizeof(uint32_t));
		SourceHandle->Read(reinterpret_cast<uint8_t*>(Indicies.data()), IndiciesLength * 4);

		mIndexBuffers.push_back(std::make_unique<Buffer>(QueueIndicies, BufferUsage::INDEX | BufferUsage::TRANSFER_DST, true, static_cast<uint32_t>(sizeof(uint32_t) * Indicies.size()), Indicies.data()));

		mIndicies.push_back(std::move(Indicies));

	}

	const int32_t SubMeshesCount = static_cast<int32_t>(mVertexBuffers.size());

	mMaterials.reserve(SubMeshesCount);
	for (int32_t i = 0; i < SubMeshesCount; ++i)
	{
		mMaterials.push_back(std::make_unique<StaticSurfaceMaterial>("StaticBasePass.vert", "StaticBasePass.frag"));
	}

}

Buffer* StaticMesh::GetVertexBuffer(int32_t Index /*= 0*/) const
{
	Assert(Index < mVertexBuffers.size());
	return mVertexBuffers[Index].get();
}

Buffer* StaticMesh::GetIndexBuffer(int32_t Index /*= 0*/) const
{
	Assert(Index < mIndexBuffers.size());
	return mIndexBuffers[Index].get();
}

uint32_t StaticMesh::GetIndiciesSize(int32_t Index /*= 0*/) const
{
	Assert(Index < mIndicies.size());
	return static_cast<uint32_t>(mIndicies[Index].size());
}

StaticMesh& StaticMesh::SetMaterial(int32_t Id, std::unique_ptr<StaticSurfaceMaterial> Material)
{
	const int32_t SubMeshesCount = static_cast<int32_t>(mVertexBuffers.size());
	if (Id < 0 || Id >= SubMeshesCount) { return *this; }

	mMaterials[Id] = std::move(Material);

	return *this;
}

StaticSurfaceMaterial* StaticMesh::GetMaterial(int32_t Id)
{
	const int32_t SubMeshesCount = static_cast<int32_t>(mVertexBuffers.size());
	if (Id < 0 || Id >= SubMeshesCount) { return nullptr; }

	return mMaterials[Id].get();
}

StaticMesh::~StaticMesh()

{
}
