#define NOMINMAX
#include "memory_manager.h"
#include "device.h"
#include <algorithm>
#include "core.h"
#include <limits>
#include "../Utilities/assert.h"

constexpr uint64_t MemoryChunkSize = 256 * 1024 * 1024;
static_assert(!(MemoryChunkSize & (MemoryChunkSize - 1)) && (MemoryChunkSize != 0), "Memory chunk size must be the power of two");

#define ALIGNMEMORY(Pointer, Aligment) (((Pointer) + (Aligment) - 1) & ~((Aligment) - 1))

MemoryChunk::MemoryChunk(uint64_t Size, uint32_t MemoryIndex, uint64_t PoolId) : mSize(Size), mMemoryIndex(MemoryIndex), mPoolId(PoolId)
{
	VkMemoryAllocateInfo AllocateInfo = {};
	AllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	AllocateInfo.allocationSize = mSize;
	AllocateInfo.memoryTypeIndex = mMemoryIndex;

	const VkDevice Device = VulkanCore::Get().GetDevice()->GetDevice();

	Assert(vkAllocateMemory(Device, &AllocateInfo, nullptr, &mMemory) == VK_SUCCESS);

	mFreeList.push_back({ 0, mSize });
}

MemoryChunk::~MemoryChunk()
{
	const VkDevice Device = VulkanCore::Get().GetDevice()->GetDevice();

	vkFreeMemory(Device, mMemory, nullptr);
}

bool MemoryChunk::Allocate(Allocation& Alloc)
{
	const uint64_t RequiredSize = Alloc.Size;
	const uint64_t RequiredAlignment = Alloc.Alignment;

	auto FreeSpace = std::find_if(mFreeList.begin(), mFreeList.end(), [RequiredSize, RequiredAlignment](auto& Elem) {

		const uint64_t AlignmentPtr = ALIGNMEMORY(Elem.Pointer, RequiredAlignment);
		const uint64_t Delta = AlignmentPtr - Elem.Pointer;

		return (Elem.Size - Delta) >= RequiredSize;
	});

	if (FreeSpace == mFreeList.end()) { return false; }

	// Fill allocation structure
	Alloc.PoolId = mPoolId;
	Alloc.Memory = mMemory;
	Alloc.MemoryIndex = mMemoryIndex;
	Alloc.Offset = ALIGNMEMORY(FreeSpace->Pointer, RequiredAlignment);

	if (FreeSpace->Size == RequiredSize)
	{
		mFreeList.erase(FreeSpace);
	}
	else
	{
		const uint64_t Delta = Alloc.Offset - FreeSpace->Pointer;

		FreeSpace->Pointer = Alloc.Offset + RequiredSize;
		FreeSpace->Size -= RequiredSize + Delta;
	}

	mOwnedAllocations.push_back(Alloc);

	return true;
}

bool MemoryChunk::Free(Allocation& Alloc)
{
	for (auto AllocIt = mOwnedAllocations.begin(); AllocIt != mOwnedAllocations.end(); ++AllocIt)
	{
		if (*AllocIt == Alloc)
		{
			mOwnedAllocations.erase(AllocIt);
			
			mFreeList.push_back({ Alloc.Offset, Alloc.Size });

			Alloc = {};

			return true;
		}
	}

	return false;
}

bool MemoryPool::Allocate(Allocation& Alloc)
{
	for (auto& Chunk : mChunks)
	{
		if (Chunk->Allocate(Alloc))
		{
			return true;
		}
	}

	MemoryChunk* NewChunk = new MemoryChunk(MemoryChunkSize, mMemoryIndex, mCurrentPoolId++);
	mChunks.push_back(NewChunk);

	return NewChunk->Allocate(Alloc);
}

bool MemoryPool::Free(Allocation& Alloc)
{
	for (auto& Chunk : mChunks)
	{
		if (Chunk->GetPoolId() == Alloc.GetPoolId())
		{
			return Chunk->Free(Alloc);
		}
	}

	return false;
}

bool MemoryManager::Startup()
{
	return true;
}

bool MemoryManager::Shutdown()
{
	for (auto& Pool : mLinearPools)
	{
		delete Pool.second;
	}

	for (auto& Pool : mNonLinearPools)
	{
		delete Pool.second;
	}

	return true;
}

bool MemoryManager::Allocate(Allocation& Alloc, VkMemoryRequirements MemReq, bool Local)
{
	VkMemoryPropertyFlags Flags = Local ? VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT : VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

	uint32_t MemoryIndex = FindMemoryIndex(MemReq, Flags);

	auto* PoolPtr = Alloc.NonLinear ? &mNonLinearPools : &mLinearPools;
	
	if (PoolPtr->find(MemoryIndex) == PoolPtr->end())
	{
		PoolPtr->insert(std::pair<uint32_t, MemoryPool*>(MemoryIndex, new MemoryPool(MemoryIndex)));
	}

	Alloc.Size = MemReq.size;
	Alloc.Alignment = MemReq.alignment;
	return (*PoolPtr)[MemoryIndex]->Allocate(Alloc);
}

bool MemoryManager::Free(Allocation& Alloc)
{
	if (!Alloc.IsValid()) { return false; }

	const uint32_t MemoryIndex = Alloc.GetMemoryIndex();
	auto* PoolPtr = Alloc.NonLinear ? &mNonLinearPools : &mLinearPools;

	if (PoolPtr->find(MemoryIndex) == PoolPtr->end())
	{
		return false;
	}

	return (*PoolPtr)[MemoryIndex]->Free(Alloc);
}

void MemoryManager::UploadData(Allocation& Alloc, const void* Data, uint32_t Size, uint32_t Offset /* = 0 */)
{
	Assert((Alloc.GetSize() - Offset) >= Size); // Overflow
	
	const VkDevice Device = VulkanCore::Get().GetDevice()->GetDevice();

	void* Memory;
	vkMapMemory(Device, Alloc.GetMemory(), Alloc.GetOffset(), Alloc.GetSize(), 0, &Memory);
	memcpy(Memory, Data, Alloc.GetSize());
	vkUnmapMemory(Device, Alloc.GetMemory());
}

uint32_t MemoryManager::FindMemoryIndex(VkMemoryRequirements MemReq, VkMemoryPropertyFlags Flags)
{
	const VkPhysicalDeviceMemoryProperties MemProp = VulkanCore::Get().GetDevice()->GetMemoryProperties();

	for (uint32_t Index = 0; Index < MemProp.memoryTypeCount; ++Index)
	{

		const bool HavePropertiesForVertexBuffer = MemReq.memoryTypeBits & (1 << Index);
		const bool CanBeMappedOnCPU = (MemProp.memoryTypes[Index].propertyFlags & Flags) == Flags;

		if (HavePropertiesForVertexBuffer && CanBeMappedOnCPU)
		{
			return Index;
		}
	}

	return std::numeric_limits<uint32_t>::max();
}
