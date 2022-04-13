#pragma once
#include <vulkan/vulkan_core.h>
namespace Graphic
{
	class GlobalSetting final
	{
	public:
		static VkFormat windowImageFormat;
		static VkColorSpaceKHR windowImageColorSpace;
		static VkPresentModeKHR windowPresentMode;
		static VkImageUsageFlags windowImageUsage;
		static VkExtent2D windowExtent;
		static uint32_t maxFrameInFlightCount;
	};
}
