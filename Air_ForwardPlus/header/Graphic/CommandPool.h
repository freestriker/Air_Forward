#pragma once
#include <vulkan/vulkan_core.h>
#include <map>
#include <string>
namespace Graphic
{
	
	class CommandPool
	{
		friend class CommandBuffer;
	private:
		VkCommandPool  _commandPool;
		std::map<std::string, Graphic::CommandBuffer> _commandBuffers;
	public:
		CommandPool();
		CommandPool(VkCommandPoolCreateFlags flag, const char* queueName);
		~CommandPool();

		void CreateCommandPool(VkCommandPoolCreateFlags flag, const char* queueName);
		void DestoryCommandPool();
		void CreateCommandBuffer(Graphic::CommandBuffer& commandBuffer, VkCommandBufferLevel level);

		Graphic::CommandBuffer& CreateCommandBuffer(const char* name, VkCommandBufferLevel level);
		Graphic::CommandBuffer& GetCommandBuffer(const char* name);
		void DestoryCommandBuffer(const char* name);
	};
}