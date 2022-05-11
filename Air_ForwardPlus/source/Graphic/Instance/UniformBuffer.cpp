#include "Graphic/Instance/UniformBuffer.h"
#include "Graphic/GlobalInstance.h"
#include "Graphic/MemoryManager.h"
#include <iostream>
#include "utils/DebugUtils.h"

Graphic::Instance::UniformBuffer::UniformBuffer(size_t size, VkMemoryPropertyFlags properties)
	: _vkBuffer(VK_NULL_HANDLE)
	, _memoryBlock(new MemoryBlock())
	, _size(size)
{
	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = static_cast<VkDeviceSize>(size);
	bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	Debug::Exception("Failed to create buffer.", vkCreateBuffer(Graphic::GlobalInstance::device, &bufferInfo, nullptr, &_vkBuffer));

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(Graphic::GlobalInstance::device, _vkBuffer, &memRequirements);

	*_memoryBlock = Graphic::GlobalInstance::memoryManager->AcquireMemoryBlock(memRequirements, properties);

	vkBindBufferMemory(Graphic::GlobalInstance::device, _vkBuffer, _memoryBlock->Memory(), _memoryBlock->Offset());
}

void Graphic::Instance::UniformBuffer::WriteBuffer(const void* data, size_t dataSize)
{
	void* transferData;
	std::unique_lock<std::mutex> lock(*_memoryBlock->Mutex());
	vkMapMemory(Graphic::GlobalInstance::device, _memoryBlock->Memory(), _memoryBlock->Offset(), _memoryBlock->Size(), 0, &transferData);
	memcpy(transferData, data, dataSize);
	vkUnmapMemory(Graphic::GlobalInstance::device, _memoryBlock->Memory());
}

VkBuffer Graphic::Instance::UniformBuffer::VkBuffer()
{
	return _vkBuffer;
}

const Graphic::MemoryBlock& Graphic::Instance::UniformBuffer::BufferMemory()
{
	return *_memoryBlock;
}

size_t Graphic::Instance::UniformBuffer::BufferSize()
{
	return _size;
}

Graphic::Instance::UniformBuffer::~UniformBuffer()
{
	vkDestroyBuffer(Graphic::GlobalInstance::device, _vkBuffer, nullptr);
	Graphic::GlobalInstance::memoryManager->ReleaseMemBlock(*_memoryBlock);

	delete _memoryBlock;
	_memoryBlock = nullptr;
	_vkBuffer = VK_NULL_HANDLE;
}
