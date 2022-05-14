#pragma once
#include <vulkan/vulkan_core.h>

namespace Graphic
{
	namespace Instance
	{
		class SwapchainImage
		{
		private:
			VkImageType _vkImageType;
			VkExtent3D _extent;
			VkFormat _vkFormat;
			VkImageUsageFlags _vkImageUsage;
			VkImageViewType _vkImageViewType;
			VkImageAspectFlags _vkImageAspect;

			VkImage _vkImage;
			VkImageView _vkImageView;

			SwapchainImage(const SwapchainImage&) = delete;
			SwapchainImage& operator=(const SwapchainImage&) = delete;
			SwapchainImage(SwapchainImage&&) = delete;
			SwapchainImage& operator=(SwapchainImage&&) = delete;

		public:
			SwapchainImage(
				VkImage swapchainImage,
				VkFormat format,
				VkExtent2D extent,
				VkImageUsageFlags usage
			);
			~SwapchainImage();
			VkImage VkImage_();
			VkImageView VkImageView_();
			VkExtent3D VkExtent3D_();
			VkFormat VkFormat_();
			VkImageSubresourceRange VkImageSubresourceRange_();
			VkImageSubresourceLayers VkImageSubresourceLayers_();
		};
	}
}