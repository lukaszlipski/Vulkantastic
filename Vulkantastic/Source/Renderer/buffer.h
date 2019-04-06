#pragma once
#include "vulkan/vulkan_core.h"
#include "memory_manager.h"

enum class BufferUsage : uint8_t
{
	VERTEX = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
	INDEX = VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
	UNIFORM = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
	STORAGE = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
	TRANSFER_DST = VK_BUFFER_USAGE_TRANSFER_DST_BIT,
	TRANSFER_SRC = VK_BUFFER_USAGE_TRANSFER_SRC_BIT
};

inline BufferUsage operator|(BufferUsage Left, BufferUsage Right)
{
	return static_cast<BufferUsage>(static_cast<uint8_t>(Left) | static_cast<uint8_t>(Right));
}

inline BufferUsage operator&(BufferUsage Left, BufferUsage Right)
{
	return static_cast<BufferUsage>(static_cast<uint8_t>(Left) & static_cast<uint8_t>(Right));
}


class Buffer
{
public:
	Buffer(const std::vector<uint32_t>& QueueIndices, BufferUsage Flags, bool GPUSide, uint32_t Size, const void* Data = nullptr);
	~Buffer();

	Buffer(const Buffer& Rhs) = delete;
	Buffer& operator=(const Buffer& Rhs) = delete;

	Buffer(Buffer&& Rhs) noexcept;
	Buffer& operator=(Buffer&& Rhs) noexcept;

	void UploadData(const void* Data, uint32_t Size, uint32_t Offset = 0);
	void CopyFromBuffer(const Buffer* Other, uint64_t Size, uint64_t SrcOffset = 0, uint64_t DstOffset = 0);

	inline VkBuffer GetBuffer() const { return mBuffer; }
	inline uint64_t GetSize() const { return mSize; }
	inline BufferUsage GetFlags() const { return mFlags; }

private:
	VkBuffer mBuffer = nullptr;
	uint32_t mSize = 0;
	std::vector<uint32_t> mQueueIndices;
	BufferUsage mFlags;
	VkMemoryRequirements mMemoryRequirements;
	Allocation mAllocation{};
	bool mGPUSide;

};

