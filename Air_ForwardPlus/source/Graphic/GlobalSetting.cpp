#include "Graphic/GlobalSetting.h"

VkFormat Graphic::GlobalSetting::windowImageFormat;
VkColorSpaceKHR Graphic::GlobalSetting::windowImageColorSpace;
VkPresentModeKHR Graphic::GlobalSetting::windowPresentMode;
VkImageUsageFlags Graphic::GlobalSetting::windowImageUsage;
VkExtent2D Graphic::GlobalSetting::windowExtent;
uint32_t Graphic::GlobalSetting::maxFrameInFlightCount;
