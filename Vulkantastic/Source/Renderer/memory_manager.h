#pragma once
#include "core.h"
#include "device.h"
#include <map>
#include <list>


struct MemoryFreeSpace
{
	uint64_t Pointer = 0;
	uint64_t Size = 0;
};

class Allocation;

class MemoryChunk
{
public:
	MemoryChunk(uint64_t Size, uint32_t MemoryIndex, uint64_t PoolId);

	~MemoryChunk();

	bool Allocate(Allocation& Alloc);

	bool Free(Allocation& Alloc);

	inline uint64_t GetPoolId() const { return mPoolId; }

private:
	VkDeviceMemory mMemory = nullptr;
	uint64_t mSize = 0;
	uint64_t mPoolId = 0;
	uint32_t mMemoryIndex = 0;

	std::list<MemoryFreeSpace> mFreeList;
	std::list<Allocation> mOwnedAllocations;

};

class MemoryPool
{
public:
	MemoryPool(uint32_t MemoryIndex)
		: mMemoryIndex(MemoryIndex)
	{ }

	~MemoryPool()
	{
		for (auto& Chunk : mChunks)
		{
			delete Chunk;
		}
	}

	bool Allocate(Allocation& Alloc);
	bool Free(Allocation& Alloc);

private:
	std::list<MemoryChunk*> mChunks;
	uint32_t mMemoryIndex = 0;
	uint64_t mCurrentPoolId = 0;

};

class MemoryManager
{
public:
	static MemoryManager& Get()
	{
		static MemoryManager* instance = new MemoryManager();
		return *instance;
	}

	bool Startup();
	bool Shutdown();

	bool Allocate(Allocation& Alloc, VkMemoryRequirements MemReq, bool Local = true);
	bool Free(Allocation& Alloc);

	void UploadData(Allocation& Alloc, const void* Data, uint32_t Size, uint32_t Offset = 0);

private:
	std::map<uint32_t, MemoryPool*> mLinearPools;
	std::map<uint32_t, MemoryPool*> mNonLinearPools;

	MemoryManager() {}

	uint32_t FindMemoryIndex(VkMemoryRequirements MemReq, VkMemoryPropertyFlags Flags);

};

class Allocation final
{
public:
	bool NonLinear = false;

	inline uint64_t GetPoolId() const { return PoolId; }
	inline uint64_t GetOffset() const { return Offset; }
	inline VkDeviceMemory GetMemory() const { return Memory; }
	inline uint32_t GetMemoryIndex() const { return MemoryIndex; }
	inline uint64_t GetSize() const { return Size; }

private:
	uint64_t Size = 0;
	uint64_t PoolId = 0;
	uint64_t Offset = 0;
	VkDeviceMemory Memory = nullptr;
	uint64_t Alignment = 0;
	uint32_t MemoryIndex = 0;

	bool operator==(Allocation& Other)
	{
		return PoolId == Other.PoolId && Size == Other.Size && Offset == Other.Offset && Memory == Other.Memory;
	}

	friend MemoryChunk;
	friend MemoryManager;
};