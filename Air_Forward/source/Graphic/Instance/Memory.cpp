#include "Graphic/Instance/Memory.h"

Graphic::Instance::Memory::Memory()
	: Memory(-1, VK_NULL_HANDLE, -1, -1, nullptr, 0)
{
}
Graphic::Instance::Memory::Memory(uint32_t memoryTypeIndex, VkDeviceMemory memory, VkDeviceSize start, VkDeviceSize size, std::mutex* mutex, VkMemoryPropertyFlags property)
	: _memoryTypeIndex(memoryTypeIndex)
	, _vkMemory(memory)
	, _offset(start)
	, _size(size)
	, _mutex(mutex)
	, _isExclusive(false)
	, _properties(property)
{
}
Graphic::Instance::Memory::Memory(bool isExclusive, uint32_t memoryTypeIndex, VkDeviceMemory memory, VkDeviceSize start, VkDeviceSize size, std::mutex* mutex, VkMemoryPropertyFlags property)
	: _memoryTypeIndex(memoryTypeIndex)
	, _vkMemory(memory)
	, _offset(start)
	, _size(size)
	, _mutex(mutex)
	, _isExclusive(isExclusive)
	, _properties(property)
{
}

Graphic::Instance::Memory::~Memory()
{
}


std::mutex& Graphic::Instance::Memory::Mutex()
{
	return *_mutex;
}

VkDeviceSize Graphic::Instance::Memory::Offset()
{
	return _offset;
}

VkDeviceSize Graphic::Instance::Memory::Size()
{
	return _size;
}

VkDeviceMemory Graphic::Instance::Memory::VkMemory()
{
	return _vkMemory;
}

VkMemoryPropertyFlags Graphic::Instance::Memory::Properties()
{
	return _properties;
}
