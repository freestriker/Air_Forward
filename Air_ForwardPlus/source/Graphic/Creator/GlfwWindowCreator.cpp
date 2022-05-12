#include "Graphic/Creator/GlfwWindowCreator.h"

Graphic::GlfwWindowCreator::GlfwWindowCreator()
	: width(800)
	, height(600)
	, title("New Window")
	, windowImageFormat(VkFormat::VK_FORMAT_B8G8R8A8_SRGB)
	, windowImageColorSpace(VkColorSpaceKHR::VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
	, windowPresentMode(VkPresentModeKHR::VK_PRESENT_MODE_MAILBOX_KHR)
	, windowImageUsage(VkImageUsageFlagBits::VK_IMAGE_USAGE_TRANSFER_DST_BIT)
	, maxFrameInFlightCount(2)
{
}

Graphic::GlfwWindowCreator::~GlfwWindowCreator()
{
}
