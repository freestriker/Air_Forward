#include "Graphic/Instance/Buffer.h"
#include "Graphic/Core/Device.h"
#include "Graphic/Manager/MemoryManager.h"
#include <iostream>
#include <Utils/Log.h>
using namespace Utils;
#include "Graphic/Instance/Memory.h"

Graphic::Instance::Buffer::Buffer(size_t size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties)
	: _vkBuffer(VK_NULL_HANDLE)
	, _memoryBlock()
	, _size(size)
	, _usage(usage)
{
	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = static_cast<VkDeviceSize>(size);
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	Log::Exception("Failed to create buffer.", vkCreateBuffer(Core::Device::VkDevice_(), &bufferInfo, nullptr, &_vkBuffer));

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(Core::Device::VkDevice_(), _vkBuffer, &memRequirements);

	_memoryBlock = Core::Device::MemoryManager().AcquireMemory(memRequirements, properties);

	vkBindBufferMemory(Core::Device::VkDevice_(), _vkBuffer, _memoryBlock.VkMemory(), _memoryBlock.Offset());
}

void Graphic::Instance::Buffer::WriteBuffer(const void* data, size_t dataSize)
{
	std::unique_lock<std::mutex> lock(_memoryBlock.Mutex());
	{
		void* transferData;
		vkMapMemory(Core::Device::VkDevice_(), _memoryBlock.VkMemory(), _memoryBlock.Offset(), _memoryBlock.Size(), 0, &transferData);
		memcpy(transferData, data, dataSize);
		vkUnmapMemory(Core::Device::VkDevice_(), _memoryBlock.VkMemory());
	}
}

Graphic::Instance::Buffer::~Buffer()
{
	vkDestroyBuffer(Core::Device::VkDevice_(), _vkBuffer, nullptr);
	Graphic::Core::Device::MemoryManager().ReleaseMemBlock(_memoryBlock);

	_vkBuffer = VK_NULL_HANDLE;
}
