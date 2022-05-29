#pragma once
#include <vulkan/vulkan_core.h>
#include <vector>
namespace Graphic
{
	namespace Instance
	{
		class Image;
		class SwapchainImage;
	}
	namespace Command
	{
		class ImageMemoryBarrier
		{
			std::vector<VkImageMemoryBarrier> _vkImageMemoryBarriers;
		public:
			ImageMemoryBarrier(Instance::Image* image, VkImageLayout oldLayout, VkImageLayout newLayout, VkAccessFlags srcAccessFlags, VkAccessFlags dstAccessFlags);;
			ImageMemoryBarrier(Instance::SwapchainImage* image, VkImageLayout oldLayout, VkImageLayout newLayout, VkAccessFlags srcAccessFlags, VkAccessFlags dstAccessFlags);;
			~ImageMemoryBarrier();
			const std::vector<VkImageMemoryBarrier>& VkImageMemoryBarriers();
		};
	}
}