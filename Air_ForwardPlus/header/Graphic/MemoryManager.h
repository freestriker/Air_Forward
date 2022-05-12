#pragma once
#include <vulkan/vulkan_core.h>
#include <map>
#include <vector>
#include <mutex>
namespace Graphic
{
	class MemoryBlock
	{
		friend class MemoryManager;
	private:
		bool _isExclusive;
		uint32_t _memoryTypeIndex;
		VkMemoryPropertyFlags _properties;
		VkDeviceMemory _vkMemory;
		VkDeviceSize _offset;
		VkDeviceSize _size;
		std::mutex* _mutex;
		MemoryBlock(uint32_t memoryTypeIndex, VkDeviceMemory memory, VkDeviceSize start, VkDeviceSize size, std::mutex* mutex, VkMemoryPropertyFlags property);
		MemoryBlock(bool isExclusive, uint32_t memoryTypeIndex, VkDeviceMemory memory, VkDeviceSize start, VkDeviceSize size, std::mutex* mutex, VkMemoryPropertyFlags property);
	public:
		MemoryBlock();
		~MemoryBlock();
		std::mutex* Mutex();
		VkDeviceSize Offset();
		VkDeviceSize Size();
		VkDeviceMemory VkMemory();
		VkMemoryPropertyFlags Properties();
	};
	class MemoryManager
	{
	private:
		class MemoryChunkUsage
		{
		public:
			VkDeviceSize offset;
			VkDeviceSize size;
			MemoryChunkUsage(VkDeviceSize start, VkDeviceSize size);
			MemoryChunkUsage();
		};
		class MemoryChunk
		{
		public:
			VkDeviceMemory memory;
			VkDeviceSize size;
			std::mutex* const mutex;
			std::map<VkDeviceSize, MemoryChunkUsage> allocated;
			std::map<VkDeviceSize, MemoryChunkUsage> unallocated;
			MemoryChunk(uint32_t typeIndex, VkDeviceSize size);
			~MemoryChunk();
		};
	private:
		std::vector<std::map<VkDeviceMemory, std::shared_ptr<MemoryChunk>>> _chunkSets;
		std::vector< VkMemoryPropertyFlags> _propertys;
		std::vector<std::mutex*> _chunkSetMutexs;
		VkDeviceSize const _defaultSize;
	public:
		MemoryManager(VkDeviceSize defaultSize);
		~MemoryManager();
		MemoryBlock AcquireMemoryBlock(VkMemoryRequirements& requirement, VkMemoryPropertyFlags properties);
		MemoryBlock GetExclusiveMemoryBlock(VkMemoryRequirements& requirement, VkMemoryPropertyFlags properties);
		void ReleaseMemBlock(MemoryBlock& memoryBlock);

	};
}