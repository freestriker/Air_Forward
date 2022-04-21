#pragma once
#include <vulkan/vulkan_core.h>
#include <map>
#include <vector>
#include <mutex>
namespace Graphic
{
	class MemoryManager
	{
	private:
		class MemoryChunkUsage
		{
		public:
			VkDeviceSize start;
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
	public:
		class MemoryBlock final
		{
			friend class MemoryManager;
		public:
			bool isExclusive;
			uint32_t memoryTypeIndex;
			VkDeviceMemory memory;
			VkDeviceSize start;
			VkDeviceSize size;
			std::mutex* mutex;
			MemoryBlock();
			~MemoryBlock();
		private:
			MemoryBlock(uint32_t memoryTypeIndex, VkDeviceMemory memory, VkDeviceSize start, VkDeviceSize size, std::mutex* mutex);
			MemoryBlock(bool isExclusive, uint32_t memoryTypeIndex, VkDeviceMemory memory, VkDeviceSize start, VkDeviceSize size, std::mutex* mutex);

		};
	private:
		std::vector<std::map<VkDeviceMemory, std::shared_ptr<MemoryChunk>>> _chunkSets;
		std::vector< VkMemoryPropertyFlags> _propertys;
		std::vector<std::mutex*> _chunkSetMutexs;
		VkDeviceSize const _defaultSize;
	public:
		MemoryManager(VkDeviceSize defaultSize);
		~MemoryManager();
		MemoryBlock GetMemoryBlock(VkMemoryRequirements requirement, VkMemoryPropertyFlags properties);
		void RecycleMemBlock(MemoryBlock memoryBlock);

	};
}