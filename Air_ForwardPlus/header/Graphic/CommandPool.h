#pragma once
#include <vulkan/vulkan_core.h>
#include <map>
#include <string>
#include <memory>
namespace Graphic
{
	
	class CommandPool
	{
		friend class CommandBuffer;
	private:
		VkCommandPool  _vkCommandPool;
		std::map<std::string, std::unique_ptr<Graphic::CommandBuffer>> _commandBuffers;

	public:
		CommandPool(VkCommandPoolCreateFlags flag, const char* queueName);

		CommandPool(const CommandPool&) = delete;
		CommandPool& operator=(const CommandPool&) = delete;
		CommandPool(CommandPool&&) = delete;
		CommandPool& operator=(CommandPool&&) = delete;

		~CommandPool();

		Graphic::CommandBuffer* const CreateCommandBuffer(const char* name, VkCommandBufferLevel level);
		Graphic::CommandBuffer* const GetCommandBuffer(const char* name);
		void DestoryCommandBuffer(const char* name);
	};
}