#pragma once
#include <vulkan/vulkan_core.h>
namespace Graphic
{
	class CommandBuffer;
	class MemoryBlock;
	namespace Asset
	{
		class UniformBuffer
		{
		private:
			VkBuffer _vkBuffer;
			MemoryBlock* _memoryBlock;
			size_t _size;

			UniformBuffer(const UniformBuffer& source) = delete;
			UniformBuffer& operator=(const UniformBuffer&) = delete;
			UniformBuffer(UniformBuffer&&) = delete;
			UniformBuffer& operator=(UniformBuffer&&) = delete;
		public:
			UniformBuffer(size_t size, VkMemoryPropertyFlags properties);
			void WriteBuffer(const void* data, size_t dataSize);
			VkBuffer Buffer();
			size_t Size();
			~UniformBuffer();
		};
	}
}