#pragma once
#include <vulkan/vulkan_core.h>
namespace Graphic
{
	namespace Instance
	{
		class Memory;
		class Image
		{
		public:
			Image(
				VkExtent2D extent,
				VkFormat format,
				VkImageTiling imageTiling,
				VkImageUsageFlagBits imageUsage,
				uint32_t mipLevels,
				VkSampleCountFlagBits sampleCount,
				VkMemoryPropertyFlagBits memoryProperty,
				VkImageViewType imageViewType,
				VkImageAspectFlagBits imageAspect
			);
			~Image();
			VkImage VkImage_();
			VkImageView VkImageView_();
			VkExtent3D VkExtent3D_();
			Memory& Memory_();
			VkImageSubresourceRange VkImageSubresourceRange_();
			VkImageSubresourceLayers VkImageSubresourceLayers_();
		private:
			VkImageType _vkImageType;
			VkExtent3D _extent;
			VkFormat _vkFormat;
			VkImageTiling _vkImageTiling;
			VkImageUsageFlagBits _vkImageUsage;
			uint32_t _mipLevels;
			VkSampleCountFlagBits _vkSampleCount;
			VkMemoryPropertyFlagBits _vkMemoryProperty;
			VkImageViewType _vkImageViewType;
			VkImageAspectFlagBits _vkImageAspect;

			VkImage _vkImage;
			VkImageView _vkImageView;
			Memory* _memory;
		};

	}
}