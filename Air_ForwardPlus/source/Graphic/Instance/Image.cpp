#include "Graphic/Instance/Image.h"
#include <Graphic/Core/Device.h>
#include "utils/Log.h"
#include "Graphic/Instance/Memory.h"
#include "Graphic/Manager/MemoryManager.h"

Graphic::Instance::Image::Image(VkExtent2D extent, VkFormat format, VkImageTiling imageTiling, VkImageUsageFlagBits imageUsage, uint32_t mipLevels, VkSampleCountFlagBits sampleCount, VkMemoryPropertyFlagBits memoryProperty, VkImageViewType imageViewType, VkImageAspectFlagBits imageAspect)
	: _vkImageType(VkImageType::VK_IMAGE_TYPE_2D)
	, _extent({ extent .width, extent .height, 1})
	, _vkFormat(format)
	, _vkImageTiling(imageTiling)
	, _vkImageUsage(imageUsage)
	, _mipLevels(mipLevels)
	, _vkSampleCount(sampleCount)
	, _vkMemoryProperty(memoryProperty)
	, _vkImageAspect(imageAspect)
	, _vkImage(VK_NULL_HANDLE)
	, _vkImageViewType(imageViewType)
	, _vkImageView(VK_NULL_HANDLE)
	, _memory(nullptr)
{
	VkImageCreateInfo imageInfo{};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = _vkImageType;
	imageInfo.extent = _extent;
	imageInfo.mipLevels = _mipLevels;
	imageInfo.arrayLayers = 1;
	imageInfo.format = _vkFormat;
	imageInfo.tiling = _vkImageTiling;
	imageInfo.initialLayout = VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = _vkImageUsage;
	imageInfo.samples = _vkSampleCount;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	Log::Exception("Failed to create image.", vkCreateImage(Core::Device::VkDevice_(), &imageInfo, nullptr, &_vkImage));

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(Core::Device::VkDevice_(), _vkImage, &memRequirements);

	_memory = new Instance::Memory();
	*_memory = Core::Device::MemoryManager().AcquireMemory(memRequirements, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	vkBindImageMemory(Core::Device::VkDevice_(), _vkImage, _memory->VkMemory(), _memory->Offset());

	VkImageViewCreateInfo viewInfo{};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = _vkImage;
	viewInfo.viewType = _vkImageViewType;
	viewInfo.format = _vkFormat;
	viewInfo.subresourceRange.aspectMask = _vkImageAspect;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = _mipLevels;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;

	Log::Exception("Failed to create image view.", vkCreateImageView(Core::Device::VkDevice_(), &viewInfo, nullptr, &_vkImageView));
}

Graphic::Instance::Image::~Image()
{
	vkDestroyImageView(Core::Device::VkDevice_(), _vkImageView, nullptr);
	vkDestroyImage(Core::Device::VkDevice_(), _vkImage, nullptr);
	Core::Device::MemoryManager().ReleaseMemBlock(*_memory);
	delete _memory;
}

VkImage Graphic::Instance::Image::VkImage_()
{
	return _vkImage;
}

VkImageView Graphic::Instance::Image::VkImageView_()
{
	return _vkImageView;
}

VkExtent3D Graphic::Instance::Image::VkExtent3D_()
{
	return _extent;
}

Graphic::Instance::Memory& Graphic::Instance::Image::Memory_()
{
	return *_memory;
}

VkFormat Graphic::Instance::Image::VkFormat_()
{
	return _vkFormat;
}

VkSampleCountFlagBits Graphic::Instance::Image::VkSampleCountFlagBits_()
{
	return _vkSampleCount;
}

VkImageSubresourceRange Graphic::Instance::Image::VkImageSubresourceRange_()
{
	VkImageSubresourceRange range{};

	range.aspectMask = _vkImageAspect;
	range.baseMipLevel = 0;
	range.levelCount = _mipLevels;
	range.baseArrayLayer = 0;
	range.layerCount = 1;

	return range;
}

VkImageSubresourceLayers Graphic::Instance::Image::VkImageSubresourceLayers_()
{
	VkImageSubresourceLayers layer{};

	layer.aspectMask = _vkImageAspect;
	layer.mipLevel = 0;
	layer.baseArrayLayer = 0;
	layer.layerCount = 1;

	return layer;
}
