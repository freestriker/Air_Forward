#include "Graphic/Manager/MemoryManager.h"
#include <Graphic/Core/Device.h>
#include "utils/Log.h"
#include "Graphic/Instance/Memory.h"

Graphic::Manager::MemoryManager::MemoryChunkUsage::MemoryChunkUsage(VkDeviceSize start, VkDeviceSize size)
	: offset(start)
	, size(size)
{

}

Graphic::Manager::MemoryManager::MemoryChunkUsage::MemoryChunkUsage()
	: MemoryChunkUsage(-1, -1)
{
}


Graphic::Manager::MemoryManager::MemoryChunk::MemoryChunk(uint32_t typeIndex, VkDeviceSize size)
	: size(size)
	, mutex(new std::mutex())
	, allocated({})
	, unallocated({ {0, Graphic::Manager::MemoryManager::MemoryChunkUsage(0, size)} })
{
	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = size;
	allocInfo.memoryTypeIndex = typeIndex;

	Log::Exception("Failed to allocate memory chunk.", vkAllocateMemory(Core::Device::VkDevice_(), &allocInfo, nullptr, &memory));
}

Graphic::Manager::MemoryManager::MemoryChunk::~MemoryChunk()
{
	vkFreeMemory(Core::Device::VkDevice_(), memory, nullptr);
	delete mutex;
}


Graphic::Manager::MemoryManager::MemoryManager(VkDeviceSize defaultSize)
	: _defaultSize(defaultSize)
{
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(Core::Device::VkPhysicalDevice_(), &memProperties);

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

Graphic::Manager::MemoryManager::~MemoryManager()
{
	_chunkSets.clear();
	for (size_t i = 0; i < _chunkSetMutexs.size(); i++)
	{
		delete _chunkSetMutexs[i];
	}
}

Graphic::Instance::Memory Graphic::Manager::MemoryManager::AcquireMemory(VkMemoryRequirements& requirement, VkMemoryPropertyFlags properties)
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

					VkDeviceSize newStart = (usagePair.second.offset + requirement.alignment - 1) & ~(requirement.alignment - 1);
					VkDeviceSize newEnd = newStart + newSize;

					VkDeviceSize oldStart = usagePair.second.offset;
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
						return Instance::Memory(i, chunkPair.second->memory, newStart, newSize, chunkPair.second->mutex, properties);
					}
				}
			}

			MemoryChunk* newChunk = new MemoryChunk(i, _defaultSize);
			newChunk->unallocated.clear();
			if(_defaultSize > newSize) newChunk->unallocated.emplace(newSize, MemoryChunkUsage(newSize, _defaultSize - newSize));
			newChunk->allocated.emplace(0, MemoryChunkUsage(0, newSize));

			_chunkSets[i].emplace(newChunk->memory, std::shared_ptr<MemoryChunk>(newChunk));
			return Instance::Memory(i, newChunk->memory, 0, newSize, newChunk->mutex, properties);
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
			Log::Exception("Failed to allocate exculsive memory.", vkAllocateMemory(Core::Device::VkDevice_(), &allocInfo, nullptr, &newMemory));
			std::mutex* newMutex = new std::mutex();

			return Instance::Memory(true, i, newMemory, 0, newSize, newMutex, properties);
		}
	}

	Log::Exception("Failed to allocate memory.");
}

Graphic::Instance::Memory Graphic::Manager::MemoryManager::AcquireExclusiveMemory(VkMemoryRequirements& requirement, VkMemoryPropertyFlags properties)
{
	VkDeviceSize newSize = (requirement.size + requirement.alignment - 1) & ~(requirement.alignment - 1);
	for (uint32_t i = 0; i < _propertys.size(); i++)
	{
		if ((requirement.memoryTypeBits & (1 << i)) && (_propertys[i] & properties) == properties)
		{
			VkMemoryAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			allocInfo.allocationSize = newSize;
			allocInfo.memoryTypeIndex = i;

			VkDeviceMemory newMemory = VK_NULL_HANDLE;
			Log::Exception("Failed to allocate exculsive memory.", vkAllocateMemory(Core::Device::VkDevice_(), &allocInfo, nullptr, &newMemory));
			std::mutex* newMutex = new std::mutex();

			return Instance::Memory(true, i, newMemory, 0, newSize, newMutex, properties);
		}
	}

	Log::Exception("Failed to allocate exculsive memory.");
}

void Graphic::Manager::MemoryManager::ReleaseMemBlock(Instance::Memory& memoryBlock)
{
	if (memoryBlock._isExclusive)
	{
		vkFreeMemory(Core::Device::VkDevice_(), memoryBlock._vkMemory, nullptr);
		delete memoryBlock._mutex;
		return;
	}
	else
	{
		std::unique_lock<std::mutex> chunkSetLock(*_chunkSetMutexs[memoryBlock._memoryTypeIndex]);
		MemoryChunk* chunk = _chunkSets[memoryBlock._memoryTypeIndex][memoryBlock._vkMemory].get();

		{
			std::unique_lock<std::mutex> chunkLock(*chunk->mutex);
			chunk->allocated.erase(memoryBlock._offset);

			VkDeviceSize recycleStart = memoryBlock._offset;
			VkDeviceSize recycleSize = memoryBlock._size;
			VkDeviceSize recycleEnd = recycleStart + recycleSize;

			bool merged = false;
			for (auto& usagePair : chunk->unallocated)
			{
				MemoryChunkUsage usage = usagePair.second;
				if (usage.offset < recycleStart)
				{
					if (usage.offset + usage.size == recycleStart)
					{
						merged = true;
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
				else
				{
					break;
				}

			}
			if (!merged)
			{
				chunk->unallocated.emplace(recycleStart, MemoryChunkUsage{ recycleStart , recycleSize });
				return;
			}
		}
		Log::Exception("Failed to release memory.");
	}
}
