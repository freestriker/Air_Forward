#pragma once
#include <vulkan/vulkan_core.h>
#include <string>
#include <vector>
namespace Graphic
{
	class CommandBuffer
	{
		friend class CommandPool;
	private:
		VkCommandBuffer _commandBuffer;
		Graphic::CommandPool* const _parentCommandPool;
	public:
		std::string name;
	private:
	public:
		CommandBuffer(const char* name, Graphic::CommandPool& commandPool, VkCommandBufferLevel level);
		CommandBuffer(const Graphic::CommandBuffer& src);
		CommandBuffer();
		~CommandBuffer();
		void Reset();
		void BeginRecord(VkCommandBufferUsageFlags flag);
		void AddPipelineBarrier(VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, std::vector<VkMemoryBarrier>& memoryBarriers, std::vector <VkBufferMemoryBarrier>& bufferMemoryBarriers, std::vector < VkImageMemoryBarrier>& imageMemoryBarriers);
		void CopyBufferToImage(VkBuffer srcBuffer, VkImage dstImage, VkImageLayout dstImageLayout, std::vector<VkBufferImageCopy>& regions);
		void EndRecord();
		void Submit(std::vector<VkSemaphore> waitSemaphores, std::vector<VkSemaphore> signalSemaphores, VkFence fence);
	};
}