#include "Graphic/Instance/Buffer.h"
#include "Graphic/GlobalInstance.h"
#include "Graphic/Manager/MemoryManager.h"
#include <iostream>
#include "utils/Log.h"
#include "Graphic/Instance/Memory.h"

Graphic::Instance::Buffer::Buffer(size_t size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties)
	: _vkBuffer(VK_NULL_HANDLE)
	, _memoryBlock(new Instance::Memory())
	, _size(size)
	, _usage(usage)
{
	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = static_cast<VkDeviceSize>(size);
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	Log::Exception("Failed to create buffer.", vkCreateBuffer(Graphic::GlobalInstance::device, &bufferInfo, nullptr, &_vkBuffer));

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(Graphic::GlobalInstance::device, _vkBuffer, &memRequirements);

	*_memoryBlock = Graphic::GlobalInstance::memoryManager->AcquireMemoryBlock(memRequirements, properties);

	vkBindBufferMemory(Graphic::GlobalInstance::device, _vkBuffer, _memoryBlock->VkMemory(), _memoryBlock->Offset());
}

void Graphic::Instance::Buffer::WriteBuffer(const void* data, size_t dataSize)
{
	void* transferData;
	std::unique_lock<std::mutex> lock(_memoryBlock->Mutex());
	vkMapMemory(Graphic::GlobalInstance::device, _memoryBlock->VkMemory(), _memoryBlock->Offset(), _memoryBlock->Size(), 0, &transferData);
	memcpy(transferData, data, dataSize);
	vkUnmapMemory(Graphic::GlobalInstance::device, _memoryBlock->VkMemory());
}

Graphic::Instance::Buffer::~Buffer()
{
	vkDestroyBuffer(Graphic::GlobalInstance::device, _vkBuffer, nullptr);
	Graphic::GlobalInstance::memoryManager->ReleaseMemBlock(*_memoryBlock);

	delete _memoryBlock;
	_memoryBlock = nullptr;
	_vkBuffer = VK_NULL_HANDLE;
}
