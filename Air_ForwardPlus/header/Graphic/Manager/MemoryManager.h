#pragma once
#include <vulkan/vulkan_core.h>
#include <map>
#include <vector>
#include <mutex>
namespace Graphic
{
	namespace Instance
	{
		class Memory;
	}
	namespace Manager
	{
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
			Instance::Memory AcquireMemory(VkMemoryRequirements& requirement, VkMemoryPropertyFlags properties);
			Instance::Memory AcquireExclusiveMemory(VkMemoryRequirements& requirement, VkMemoryPropertyFlags properties);
			void ReleaseMemBlock(Instance::Memory& memoryBlock);

		};
	}
}