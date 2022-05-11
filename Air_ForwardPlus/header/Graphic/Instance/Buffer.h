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
			VkBuffer VkBuffer();
			const MemoryBlock& Memory();
			size_t Size();
			~Buffer();
		};
	}
}