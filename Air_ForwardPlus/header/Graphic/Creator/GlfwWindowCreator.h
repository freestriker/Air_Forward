#pragma once
#include <vulkan/vulkan_core.h>

namespace Graphic
{
	class GlfwWindowCreator final
	{
		friend class GlobalInstance;
	public:
		uint32_t width;
		uint32_t height;
		const char* title;

		VkFormat windowImageFormat;
		VkColorSpaceKHR windowImageColorSpace;
		VkPresentModeKHR windowPresentMode;
		VkImageUsageFlags windowImageUsage;
		uint32_t maxFrameInFlightCount;

	public:
		GlfwWindowCreator();
		~GlfwWindowCreator();
	private:

	};
}