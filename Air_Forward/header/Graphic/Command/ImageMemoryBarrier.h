#pragma once
#include <vulkan/vulkan_core.h>
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
			VkImageMemoryBarrier _vkImageMemoryBarrier;
		public:
			ImageMemoryBarrier(Instance::Image* image, VkImageLayout oldLayout, VkImageLayout newLayout, VkAccessFlags srcAccessFlags, VkAccessFlags dstAccessFlags);;
			ImageMemoryBarrier(Instance::SwapchainImage* image, VkImageLayout oldLayout, VkImageLayout newLayout, VkAccessFlags srcAccessFlags, VkAccessFlags dstAccessFlags);;
			~ImageMemoryBarrier();
			VkImageMemoryBarrier VkImageMemoryBarrier_();
		};
	}
}