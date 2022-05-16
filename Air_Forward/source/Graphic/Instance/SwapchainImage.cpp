#include "Graphic/Instance/SwapchainImage.h"
#include <Graphic/Core/Device.h>
#include <Utils/Log.h>
using namespace Utils;

Graphic::Instance::SwapchainImage::SwapchainImage(VkImage swapchainImage, VkFormat format, VkExtent2D extent, VkImageUsageFlags usage)
	: _vkImage(swapchainImage)
	, _vkFormat(format)
	, _extent({ extent .width, extent .height, 1})
	, _vkImageUsage(usage)
	, _vkImageType(VkImageType::VK_IMAGE_TYPE_2D)
	, _vkImageViewType(VkImageViewType::VK_IMAGE_VIEW_TYPE_2D)
	, _vkImageAspect(VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT)
	, _vkImageView(VK_NULL_HANDLE)
{
	//VkImageViewCreateInfo viewInfo{};
	//viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	//viewInfo.image = _vkImage;
	//viewInfo.viewType = _vkImageViewType;
	//viewInfo.format = _vkFormat;
	//viewInfo.subresourceRange.aspectMask = _vkImageAspect;
	//viewInfo.subresourceRange.baseMipLevel = 0;
	//viewInfo.subresourceRange.levelCount = 1;
	//viewInfo.subresourceRange.baseArrayLayer = 0;
	//viewInfo.subresourceRange.layerCount = 1;

	//Log::Exception("Failed to create image view.", vkCreateImageView(Graphic::GlobalInstance::device, &viewInfo, nullptr, &_vkImageView));

}

Graphic::Instance::SwapchainImage::~SwapchainImage()
{
}
VkImage Graphic::Instance::SwapchainImage::VkImage_()
{
	return _vkImage;
}

VkImageView Graphic::Instance::SwapchainImage::VkImageView_()
{
	return _vkImageView;
}

VkExtent3D Graphic::Instance::SwapchainImage::VkExtent3D_()
{
	return _extent;
}

VkFormat Graphic::Instance::SwapchainImage::VkFormat_()
{
	return _vkFormat;
}

VkImageSubresourceRange Graphic::Instance::SwapchainImage::VkImageSubresourceRange_()
{
	VkImageSubresourceRange range{};

	range.aspectMask = _vkImageAspect;
	range.baseMipLevel = 0;
	range.levelCount = 1;
	range.baseArrayLayer = 0;
	range.layerCount = 1;

	return range;
}

VkImageSubresourceLayers Graphic::Instance::SwapchainImage::VkImageSubresourceLayers_()
{
	VkImageSubresourceLayers layer{};

	layer.aspectMask = _vkImageAspect;
	layer.mipLevel = 0;
	layer.baseArrayLayer = 0;
	layer.layerCount = 1;

	return layer;
}
