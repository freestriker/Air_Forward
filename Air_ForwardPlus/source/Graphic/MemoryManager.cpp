#include "Graphic/MemoryManager.h"
#include <Graphic/GlobalInstance.h>

Graphic::MemoryManager::MemoryChunkUsage::MemoryChunkUsage(VkDeviceSize start, VkDeviceSize size)
	: start(start)
	, size(size)
{

}

Graphic::MemoryManager::MemoryChunkUsage::MemoryChunkUsage()
	: MemoryChunkUsage(-1, -1)
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
Graphic::MemoryManager::MemoryBlock::MemoryBlock(uint32_t memoryTypeIndex, VkDeviceMemory memory, VkDeviceSize start, VkDeviceSize size, std::mutex* mutex)
	: memoryTypeIndex(memoryTypeIndex)
	, memory(memory)
	, start(start)
	, size(size)
	, mutex(mutex)
	, isExclusive(false)
{
}
Graphic::MemoryManager::MemoryBlock::MemoryBlock(bool isExclusive, uint32_t memoryTypeIndex, VkDeviceMemory memory, VkDeviceSize start, VkDeviceSize size, std::mutex* mutex)
	: memoryTypeIndex(memoryTypeIndex)
	, memory(memory)
	, start(start)
	, size(size)
	, mutex(mutex)
	, isExclusive(isExclusive)
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
	VkDeviceSize newSize = (requirement.size + requirement.alignment - 1) & ~(requirement.alignment - 1);
	if (newSize > _defaultSize) goto EXCLUSIVE;
	for (uint32_t i = 0; i < _propertys.size(); i++)
	{
		if ((requirement.memoryTypeBits & (1 << i)) && (_propertys[i] & properties) == properties)
		{
			std::unique_lock<std::mutex> chunkSetLock(*_chunkSetMutexs[i]);
			
			for (auto& chunkPair : _chunkSets[i])
			{
				std::unique_lock<std::mutex> chunkLock(*chunkPair.second->mutex);

				for (auto& usagePair : chunkPair.second->unallocated)
				{
					if (usagePair.second.size < requirement.size) continue;

					VkDeviceSize newStart = (usagePair.second.start + requirement.alignment - 1) & ~(requirement.alignment - 1);
					VkDeviceSize newEnd = newStart + newSize;

					VkDeviceSize oldStart = usagePair.second.start;
					VkDeviceSize oldSize = usagePair.second.size;
					VkDeviceSize oldEnd = oldStart + oldSize;

					if (newEnd <= oldEnd)
					{
						if (newStart == oldStart)
						{
							chunkPair.second->unallocated.erase(oldStart);
						}
						else
						{
							usagePair.second.size = newStart - oldStart;
						}
						if (oldEnd > newEnd) chunkPair.second->unallocated.emplace(newEnd, MemoryChunkUsage(newEnd, oldEnd - newEnd));
						chunkPair.second->allocated.emplace(newStart, MemoryChunkUsage(newStart, newSize));
						return MemoryBlock(i, chunkPair.second->memory, newStart, newSize, chunkPair.second->mutex);
					}
				}
			}

			MemoryChunk* newChunk = new MemoryChunk(i, _defaultSize);
			newChunk->unallocated.clear();
			if(_defaultSize > newSize) newChunk->unallocated.emplace(newSize, MemoryChunkUsage(newSize, _defaultSize - newSize));
			newChunk->allocated.emplace(0, MemoryChunkUsage(0, newSize));

			_chunkSets[i].emplace(newChunk->memory, std::shared_ptr<MemoryChunk>(newChunk));
			return MemoryBlock(i, newChunk->memory, 0, newSize, newChunk->mutex);
		}
	}

EXCLUSIVE:
	for (uint32_t i = 0; i < _propertys.size(); i++)
	{
		if ((requirement.memoryTypeBits & (1 << i)) && (_propertys[i] & properties) == properties)
		{
			VkMemoryAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			allocInfo.allocationSize = newSize;
			allocInfo.memoryTypeIndex = i;

			VkDeviceMemory newMemory = VK_NULL_HANDLE;
			if (vkAllocateMemory(Graphic::GlobalInstance::device, &allocInfo, nullptr, &newMemory) != VK_SUCCESS)
			{
				throw std::runtime_error("failed to allocate buffer memory!");
			}
			std::mutex* newMutex = new std::mutex();

			return MemoryBlock(true, i, newMemory, 0, newSize, newMutex);
		}
	}

	throw std::runtime_error("failed to allocate memory.");
}

void Graphic::MemoryManager::RecycleMemBlock(MemoryBlock memoryBlock)
{
	if (memoryBlock.isExclusive)
	{
		vkFreeMemory(Graphic::GlobalInstance::device, memoryBlock.memory, nullptr);
		delete memoryBlock.mutex;
		return;
	}
	else
	{
		std::unique_lock<std::mutex> chunkSetLock(*_chunkSetMutexs[memoryBlock.memoryTypeIndex]);
		MemoryChunk* chunk = _chunkSets[memoryBlock.memoryTypeIndex][memoryBlock.memory].get();

		{
			std::unique_lock<std::mutex> chunkLock(*chunk->mutex);
			chunk->allocated.erase(memoryBlock.start);

			VkDeviceSize recycleStart = memoryBlock.start;
			VkDeviceSize recycleSize = memoryBlock.size;
			VkDeviceSize recycleEnd = recycleStart + recycleSize;

			for (auto& usagePair : chunk->unallocated)
			{
				MemoryChunkUsage usage = usagePair.second;
				if (usage.start + usage.size == recycleStart)
				{
					if (chunk->unallocated.count(recycleEnd))
					{
						usagePair.second.size = usage.size + recycleSize + chunk->unallocated[recycleEnd].size;
						chunk->unallocated.erase(recycleEnd);
					}
					else
					{
						usagePair.second.size = usage.size + recycleSize;
					}
					return;
				}
			}
		}
		throw std::runtime_error("failed to recycle memory.");
	}
}
