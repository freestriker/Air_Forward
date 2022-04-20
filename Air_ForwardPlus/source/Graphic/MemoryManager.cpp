#include "Graphic/MemoryManager.h"
#include <Graphic/GlobalInstance.h>

Graphic::MemoryManager::MemoryChunkUsage::MemoryChunkUsage(VkDeviceSize start, VkDeviceSize size)
	: start(start)
	, size(size)
{

}

Graphic::MemoryManager::MemoryChunk::MemoryChunk(uint32_t typeIndex, VkDeviceSize size)
	: size(size)
	, mutex(new std::mutex())
	, allocated({})
	, unallocated({ {0, Graphic::MemoryManager::MemoryChunkUsage(0, size)} })
{
	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = size;
	allocInfo.memoryTypeIndex = typeIndex;

	if (vkAllocateMemory(Graphic::GlobalInstance::device, &allocInfo, nullptr, &memory) != VK_SUCCESS) 
	{
		throw std::runtime_error("failed to allocate buffer memory!");
	}
}

Graphic::MemoryManager::MemoryChunk::~MemoryChunk()
{
	vkFreeMemory(Graphic::GlobalInstance::device, memory, nullptr);
	delete mutex;
}

Graphic::MemoryManager::MemoryBlock::MemoryBlock()
	: MemoryBlock(-1, VK_NULL_HANDLE, -1, -1, nullptr)
{
}
Graphic::MemoryManager::MemoryBlock::MemoryBlock(uint32_t const memoryTypeIndex, VkDeviceMemory const memory, VkDeviceSize const start, VkDeviceSize const size, std::mutex* const mutex)
	: memoryTypeIndex(memoryTypeIndex)
	, memory(memory)
	, start(start)
	, size(size)
	, mutex(mutex)
{
}

Graphic::MemoryManager::MemoryBlock::~MemoryBlock()
{
}

Graphic::MemoryManager::MemoryManager(VkDeviceSize defaultSize)
	: _defaultSize(defaultSize)
{
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(Graphic::GlobalInstance::physicalDevice, &memProperties);

	_chunkSets.resize(memProperties.memoryTypeCount);

	_propertys.resize(memProperties.memoryTypeCount);
	for (size_t i = 0; i < _propertys.size(); i++)
	{
		_propertys[i] = memProperties.memoryTypes[i].propertyFlags;
	}

	_chunkSetMutexs.resize(memProperties.memoryTypeCount);
	for (size_t i = 0; i < _chunkSetMutexs.size(); i++)
	{
		_chunkSetMutexs[i] = new std::mutex();
	}

}

Graphic::MemoryManager::~MemoryManager()
{
	_chunkSets.clear();
	for (size_t i = 0; i < _chunkSetMutexs.size(); i++)
	{
		delete _chunkSetMutexs[i];
	}
}

Graphic::MemoryManager::MemoryBlock Graphic::MemoryManager::GetMemoryBlock(VkMemoryRequirements requirement, VkMemoryPropertyFlags properties)
{
	for (uint32_t i = 0; i < _propertys.size(); i++)
	{
		if ((requirement.memoryTypeBits & (1 << i)) && (_propertys[i] & properties) == properties)
		{
			std::unique_lock<std::mutex> chunkSetLock(*_chunkSetMutexs[i]);
			
			for (auto& chunkPair : _chunkSets[i])
			{
				std::unique_lock<std::mutex> chunkLock(*chunkPair.second.mutex);

				for (auto& usagePair : chunkPair.second.unallocated)
				{
					VkDeviceSize downStart = usagePair.second.start % requirement.alignment;
					if(usagePair.second.start)
				}
			}
		}
	}
}

void Graphic::MemoryManager::RecycleMemBlock(MemoryBlock memoryBlock)
{
}
