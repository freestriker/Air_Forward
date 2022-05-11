#pragma once
#include <vulkan/vulkan_core.h>
namespace Graphic
{
	class CommandBuffer;
	class MemoryBlock;
	namespace Instance
	{
		class Buffer
		{
		private:
			VkBuffer _vkBuffer;
			MemoryBlock* _memoryBlock;
			size_t _size;
			VkBufferUsageFlags _usage;

			Buffer(const Buffer& source) = delete;
			Buffer& operator=(const Buffer&) = delete;
			Buffer(Buffer&&) = delete;
			Buffer& operator=(Buffer&&) = delete;
		public:
			Buffer(size_t size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
			void WriteBuffer(const void* data, size_t dataSize);
			inline VkBuffer VkBuffer();
			inline MemoryBlock& Memory();
			inline size_t Size();
			inline size_t Offset();
			~Buffer();
		};
	}
}

inline VkBuffer Graphic::Instance::Buffer::VkBuffer()
{
	return _vkBuffer;
}

inline Graphic::MemoryBlock& Graphic::Instance::Buffer::Memory()
{
	return *_memoryBlock;
}

inline size_t Graphic::Instance::Buffer::Size()
{
	return _size;
}
inline size_t Graphic::Instance::Buffer::Offset()
{
	return 0;
}
