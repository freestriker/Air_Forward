#include "Logic/Component/Renderer/BackgroundRenderer.h"
#include <rttr/registration>
#include <Graphic/Core/Window.h>
RTTR_REGISTRATION
{
    using namespace rttr;
    registration::class_<Logic::Component::Renderer::BackgroundRenderer>("Logic::Component::Renderer::BackgroundRenderer");
}

void Logic::Component::Renderer::BackgroundRenderer::OnStart()
{
	_temporaryImage = Graphic::Instance::Image::Create2DImage(
		Graphic::Core::Window::VkExtent2D_()
		, VkFormat::VK_FORMAT_D32_SFLOAT
		, VkImageUsageFlagBits::VK_IMAGE_USAGE_SAMPLED_BIT | VkImageUsageFlagBits::VK_IMAGE_USAGE_TRANSFER_DST_BIT | VkImageUsageFlagBits::VK_IMAGE_USAGE_TRANSFER_SRC_BIT
		, VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		, VkImageAspectFlagBits::VK_IMAGE_ASPECT_DEPTH_BIT
	);
	_temporaryImageSampler = new Graphic::Instance::ImageSampler
	(
		VkFilter::VK_FILTER_LINEAR,
		VkSamplerMipmapMode::VK_SAMPLER_MIPMAP_MODE_LINEAR,
		VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
		0.0f,
		VkBorderColor::VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE
	);
}

Logic::Component::Renderer::BackgroundRenderer::BackgroundRenderer()
    : Renderer()
    , _temporaryImage(nullptr)
    , _temporaryImageSampler(nullptr)
{
}

Logic::Component::Renderer::BackgroundRenderer::~BackgroundRenderer()
{
}
