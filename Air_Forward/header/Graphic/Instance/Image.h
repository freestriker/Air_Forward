#pragma once
#include <vulkan/vulkan_core.h>
#include <vector>
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
			VkFormat VkFormat_();
			VkSampleCountFlagBits VkSampleCountFlagBits_();
			std::vector<VkImageSubresourceRange> VkImageSubresourceRanges_();
			std::vector<VkImageSubresourceLayers> VkImageSubresourceLayers_();
			uint32_t LayerCount();
			size_t PerLayerSize();

			static Image* CreateCubeImage(
				VkExtent2D extent,
				VkFormat format,
				VkImageUsageFlags imageUsage,
				VkMemoryPropertyFlags memoryProperty
				);
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
			uint32_t _layerCount;
			size_t _perLayerSize;

			VkImage _vkImage;
			VkImageView _vkImageView;
			Memory* _memory;

			Image();
			Image(const Image&) = delete;
			Image& operator=(const Image&) = delete;
			Image(Image&&) = delete;
			Image& operator=(Image&&) = delete;
		};

	}
}