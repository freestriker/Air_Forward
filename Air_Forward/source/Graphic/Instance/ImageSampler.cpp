#include "Graphic/Instance/ImageSampler.h"
#include <Graphic/Core/Device.h>
#include <utils/Log.h>
#include <algorithm>

Graphic::Instance::ImageSampler::ImageSampler(VkFilter magFilter, VkFilter minFilter, VkSamplerMipmapMode mipmapMode, VkSamplerAddressMode addressModeU, VkSamplerAddressMode addressModeV, VkSamplerAddressMode addressModeW, float maxAnisotropy, VkBorderColor borderColor)
	: _magFilter(magFilter)
	, _minFilter(minFilter)
	, _mipmapMode(mipmapMode)
	, _addressModeU(addressModeU)
	, _addressModeV(addressModeV)
	, _addressModeW(addressModeW)
	, _maxAnisotropy(std::min(maxAnisotropy, 1.0f))
	, _anisotropyEnable(maxAnisotropy < 1.0f ? VK_FALSE : VK_TRUE)
	, _borderColor(borderColor)
{
	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = _magFilter;
	samplerInfo.minFilter = _minFilter;
	samplerInfo.addressModeU = _addressModeU;
	samplerInfo.addressModeV = _addressModeV;
	samplerInfo.addressModeW = _addressModeW;
	samplerInfo.anisotropyEnable = _anisotropyEnable;
	samplerInfo.maxAnisotropy = _maxAnisotropy;
	samplerInfo.borderColor = _borderColor;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = _mipmapMode;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = 0.0f;
	samplerInfo.mipLodBias = 0.0f;

	Log::Exception("Failed to create sampler.", vkCreateSampler(Core::Device::VkDevice_(), &samplerInfo, nullptr, &_vkSampler));

}

Graphic::Instance::ImageSampler::ImageSampler(VkFilter filter, VkSamplerMipmapMode mipmapMode, VkSamplerAddressMode addressMode, float maxAnisotropy, VkBorderColor borderColor)
	: ImageSampler(filter, filter, mipmapMode, addressMode, addressMode, addressMode, maxAnisotropy, borderColor)
{
}

Graphic::Instance::ImageSampler::~ImageSampler()
{
	vkDestroySampler(Core::Device::VkDevice_(), _vkSampler, nullptr);
}

VkSampler Graphic::Instance::ImageSampler::VkSampler_()
{
	return _vkSampler;
}
