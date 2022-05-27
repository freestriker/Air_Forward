#include "Graphic/Command/ImageMemoryBarrier.h"
#include "Graphic/Instance/Image.h"
#include "Graphic/Instance/SwapchainImage.h"

Graphic::Command::ImageMemoryBarrier::ImageMemoryBarrier(Instance::Image* image, VkImageLayout oldLayout, VkImageLayout newLayout, VkAccessFlags srcAccessFlags, VkAccessFlags dstAccessFlags)
	: _vkImageMemoryBarriers()
{
	auto layerCount = image->LayerCount();
	auto ranges = image->VkImageSubresourceRanges_();

	_vkImageMemoryBarriers.resize(layerCount);

	for (uint32_t i = 0; i < layerCount; i++)
	{
		auto& _vkImageMemoryBarrier = _vkImageMemoryBarriers[i];

		_vkImageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		_vkImageMemoryBarrier.oldLayout = oldLayout;
		_vkImageMemoryBarrier.newLayout = newLayout;
		_vkImageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		_vkImageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		_vkImageMemoryBarrier.image = image->VkImage_();
		_vkImageMemoryBarrier.subresourceRange = ranges[i];
		_vkImageMemoryBarrier.srcAccessMask = srcAccessFlags;
		_vkImageMemoryBarrier.dstAccessMask = dstAccessFlags;
	}
}

Graphic::Command::ImageMemoryBarrier::ImageMemoryBarrier(Instance::SwapchainImage* image, VkImageLayout oldLayout, VkImageLayout newLayout, VkAccessFlags srcAccessFlags, VkAccessFlags dstAccessFlags)
	: _vkImageMemoryBarriers()
{
	_vkImageMemoryBarriers.resize(1);
	auto& _vkImageMemoryBarrier = _vkImageMemoryBarriers[0];

	_vkImageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	_vkImageMemoryBarrier.oldLayout = oldLayout;
	_vkImageMemoryBarrier.newLayout = newLayout;
	_vkImageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	_vkImageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	_vkImageMemoryBarrier.image = image->VkImage_();
	_vkImageMemoryBarrier.subresourceRange = image->VkImageSubresourceRange_();
	_vkImageMemoryBarrier.srcAccessMask = srcAccessFlags;
	_vkImageMemoryBarrier.dstAccessMask = dstAccessFlags;
}

Graphic::Command::ImageMemoryBarrier::~ImageMemoryBarrier()
{
}

const std::vector<VkImageMemoryBarrier>& Graphic::Command::ImageMemoryBarrier::VkImageMemoryBarriers()
{
	return _vkImageMemoryBarriers;
}