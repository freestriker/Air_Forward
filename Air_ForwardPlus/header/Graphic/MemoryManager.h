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
			uint32_t const memoryTypeIndex;
			VkDeviceMemory const memory;
			VkDeviceSize const start;
			VkDeviceSize const size;
			std::mutex* const mutex;
			MemoryBlock();
			~MemoryBlock();
		private:
			MemoryBlock(uint32_t const memoryTypeIndex, VkDeviceMemory const memory, VkDeviceSize const start, VkDeviceSize const size, std::mutex* const mutex);

		};
	private:
		std::vector<std::map<VkDeviceMemory, MemoryChunk>> _chunkSets;
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