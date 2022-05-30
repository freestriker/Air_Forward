#include "Graphic/Instance/Image.h"
#include <Graphic/Core/Device.h>
#include <Utils/Log.h>
using namespace Utils;
#include "Graphic/Instance/Memory.h"
#include "Graphic/Manager/MemoryManager.h"

Graphic::Instance::Image::Image()
{

}

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
	, _layerCount(1)
	, _perLayerSize(extent.width * extent.height * 4)
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
	imageInfo.flags = 0;

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
	viewInfo.subresourceRange.layerCount = _layerCount;

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

std::vector<VkImageSubresourceRange> Graphic::Instance::Image::VkImageSubresourceRanges_()
{
	std::vector<VkImageSubresourceRange> targets = std::vector<VkImageSubresourceRange>(_layerCount);
	for (uint32_t i = 0; i < _layerCount; i++)
	{
		auto& range = targets[i];

		range.aspectMask = _vkImageAspect;
		range.baseMipLevel = 0;
		range.levelCount = _mipLevels;
		range.baseArrayLayer = static_cast<uint32_t>(i);
		range.layerCount = 1;
	}

	return targets;
}

std::vector<VkImageSubresourceLayers> Graphic::Instance::Image::VkImageSubresourceLayers_()
{
	std::vector<VkImageSubresourceLayers> targets = std::vector<VkImageSubresourceLayers>(_layerCount);
	for (uint32_t i = 0; i < _layerCount; i++)
	{
		auto& layer = targets[i];

		layer.aspectMask = _vkImageAspect;
		layer.mipLevel = 0;
		layer.baseArrayLayer = static_cast<uint32_t>(i);
		layer.layerCount = 1;
	}
	return targets;
}

uint32_t Graphic::Instance::Image::LayerCount()
{
	return _layerCount;
}

size_t Graphic::Instance::Image::PerLayerSize()
{
	return _perLayerSize;
}

Graphic::Instance::Image* Graphic::Instance::Image::CreateCubeImage(VkExtent2D extent, VkFormat format, VkImageUsageFlags imageUsage, VkMemoryPropertyFlags memoryProperty)
{
	VkImageCreateInfo imageInfo{};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VkImageType::VK_IMAGE_TYPE_2D;
	imageInfo.extent = { extent.width, extent.height, 1 };
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 6;
	imageInfo.format = format;
	imageInfo.tiling = VkImageTiling::VK_IMAGE_TILING_OPTIMAL;
	imageInfo.initialLayout = VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = imageUsage;
	imageInfo.samples = VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.flags = VkImageCreateFlagBits::VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

	VkImage newVkImage = VK_NULL_HANDLE;
	Log::Exception("Failed to create image.", vkCreateImage(Core::Device::VkDevice_(), &imageInfo, nullptr, &newVkImage));

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(Core::Device::VkDevice_(), newVkImage, &memRequirements);

	auto newMemory = new Instance::Memory();
	*newMemory = Core::Device::MemoryManager().AcquireMemory(memRequirements, memoryProperty);
	vkBindImageMemory(Core::Device::VkDevice_(), newVkImage, newMemory->VkMemory(), newMemory->Offset());

	VkImageViewCreateInfo viewInfo{};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = newVkImage;
	viewInfo.viewType = VkImageViewType::VK_IMAGE_VIEW_TYPE_CUBE;
	viewInfo.format = format;
	viewInfo.subresourceRange.aspectMask = VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 6;

	VkImageView newImageView = VK_NULL_HANDLE;
	Log::Exception("Failed to create image view.", vkCreateImageView(Core::Device::VkDevice_(), &viewInfo, nullptr, &newImageView));

	Graphic::Instance::Image* newImage = new Graphic::Instance::Image();
	newImage->_vkImageType = VkImageType::VK_IMAGE_TYPE_2D;
	newImage->_extent = { extent.width, extent.height, 1 };
	newImage->_vkFormat = format;
	newImage->_vkImageTiling = VkImageTiling::VK_IMAGE_TILING_OPTIMAL;
	newImage->_vkImageUsage = static_cast<VkImageUsageFlagBits>(imageUsage);
	newImage->_mipLevels = 1;
	newImage->_vkSampleCount = VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT;
	newImage->_vkMemoryProperty = static_cast<VkMemoryPropertyFlagBits>(memoryProperty);
	newImage->_vkImageViewType = VkImageViewType::VK_IMAGE_VIEW_TYPE_CUBE;
	newImage->_vkImageAspect = VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT;
	newImage->_vkImage = newVkImage;
	newImage->_vkImageView = newImageView;
	newImage->_memory = newMemory;
	newImage->_layerCount = 6;
	newImage->_perLayerSize = extent.width * extent.height * 4;

	return newImage;
}

Graphic::Instance::Image* Graphic::Instance::Image::Create2DImage(VkExtent2D extent, VkFormat format, VkImageUsageFlags imageUsage, VkMemoryPropertyFlags memoryProperty, VkImageAspectFlags aspect)
{
	VkImageCreateInfo imageInfo{};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VkImageType::VK_IMAGE_TYPE_2D;
	imageInfo.extent = { extent.width, extent.height, 1 };
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.format = format;
	imageInfo.tiling = VkImageTiling::VK_IMAGE_TILING_OPTIMAL;
	imageInfo.initialLayout = VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = imageUsage;
	imageInfo.samples = VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.flags = 0;

	VkImage newVkImage = VK_NULL_HANDLE;
	Log::Exception("Failed to create image.", vkCreateImage(Core::Device::VkDevice_(), &imageInfo, nullptr, &newVkImage));

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(Core::Device::VkDevice_(), newVkImage, &memRequirements);

	auto newMemory = new Instance::Memory();
	*newMemory = Core::Device::MemoryManager().AcquireMemory(memRequirements, memoryProperty);
	vkBindImageMemory(Core::Device::VkDevice_(), newVkImage, newMemory->VkMemory(), newMemory->Offset());

	VkImageViewCreateInfo viewInfo{};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = newVkImage;
	viewInfo.viewType = VkImageViewType::VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = format;
	viewInfo.subresourceRange.aspectMask = aspect;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;

	VkImageView newImageView = VK_NULL_HANDLE;
	Log::Exception("Failed to create image view.", vkCreateImageView(Core::Device::VkDevice_(), &viewInfo, nullptr, &newImageView));

	Graphic::Instance::Image* newImage = new Graphic::Instance::Image();
	newImage->_vkImageType = VkImageType::VK_IMAGE_TYPE_2D;
	newImage->_extent = { extent.width, extent.height, 1 };
	newImage->_vkFormat = format;
	newImage->_vkImageTiling = VkImageTiling::VK_IMAGE_TILING_OPTIMAL;
	newImage->_vkImageUsage = static_cast<VkImageUsageFlagBits>(imageUsage);
	newImage->_mipLevels = 1;
	newImage->_vkSampleCount = VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT;
	newImage->_vkMemoryProperty = static_cast<VkMemoryPropertyFlagBits>(memoryProperty);
	newImage->_vkImageViewType = VkImageViewType::VK_IMAGE_VIEW_TYPE_2D;
	newImage->_vkImageAspect = static_cast<VkImageAspectFlagBits>(aspect);
	newImage->_vkImage = newVkImage;
	newImage->_vkImageView = newImageView;
	newImage->_memory = newMemory;
	newImage->_layerCount = 1;
	newImage->_perLayerSize = extent.width * extent.height * 4;

	return newImage;
}
