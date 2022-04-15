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
		Graphic::CommandPool* const _parentCommandPool;
		VkCommandBuffer _vkCommandBuffer;
		VkFence _vkFence;
	public:
		std::string const name;
	private:
		CommandBuffer(const char* name, Graphic::CommandPool* const commandPool, VkCommandBufferLevel level);
	public:
		CommandBuffer();
		~CommandBuffer();

		void Reset();
		void BeginRecord(VkCommandBufferUsageFlags flag);
		void AddPipelineBarrier(VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, std::vector<VkMemoryBarrier>& memoryBarriers, std::vector <VkBufferMemoryBarrier>& bufferMemoryBarriers, std::vector < VkImageMemoryBarrier>& imageMemoryBarriers);
		void CopyBufferToImage(VkBuffer srcBuffer, VkImage dstImage, VkImageLayout dstImageLayout, std::vector<VkBufferImageCopy>& regions);
		void EndRecord();
		void Submit(std::vector<VkSemaphore> waitSemaphores, std::vector<VkSemaphore> signalSemaphores, VkFence fence);
		void WaitForFinish();
	};
}