#include "Graphic/Core/Window.h"
#include "Graphic/Core/Instance.h"
#include "Graphic/Core/Device.h"
#include <Utils/Log.h>
using namespace Utils;
#include <algorithm>
#include "Graphic/Instance/SwapchainImage.h"

VkFormat Graphic::Core::Window::_imageFormat;
VkColorSpaceKHR Graphic::Core::Window::_imageColorSpace;
VkPresentModeKHR Graphic::Core::Window::_presentMode;
VkImageUsageFlags Graphic::Core::Window::_imageUsage;
VkExtent2D Graphic::Core::Window::_extent;
uint32_t Graphic::Core::Window::_maxFrameInFlightCount;
GLFWwindow* Graphic::Core::Window::_glfwWindow = nullptr;
VkSurfaceKHR Graphic::Core::Window::_vkSurface = VK_NULL_HANDLE;
VkSwapchainKHR Graphic::Core::Window::_vkSwapchain = VK_NULL_HANDLE;
std::vector<Graphic::Instance::SwapchainImage*> Graphic::Core::Window::_swapchainImages = std::vector<Graphic::Instance::SwapchainImage*>();

Graphic::Core::Window::WindowCreator::WindowCreator()
	: width(800)
	, height(450)
	, title("New Window")
	, windowImageFormat(VkFormat::VK_FORMAT_B8G8R8A8_SRGB)
	, windowImageColorSpace(VkColorSpaceKHR::VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
	, windowPresentMode(VkPresentModeKHR::VK_PRESENT_MODE_MAILBOX_KHR)
	, windowImageUsage(VkImageUsageFlagBits::VK_IMAGE_USAGE_TRANSFER_DST_BIT)
	, maxFrameInFlightCount(2)
{
}

Graphic::Core::Window::WindowCreator::~WindowCreator()
{
}

void Graphic::Core::Window::Create(WindowCreator& creator)
{
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	
	_extent = { creator.width, creator.height };
	_imageColorSpace = creator.windowImageColorSpace;
	_imageFormat = creator.windowImageFormat;
	_imageUsage = creator.windowImageUsage;
	_presentMode = creator.windowPresentMode;
	_maxFrameInFlightCount = creator.maxFrameInFlightCount;
	_glfwWindow = glfwCreateWindow(creator.width, creator.height, creator.title.c_str(), nullptr, nullptr);
}

void Graphic::Core::Window::_CreateSurface()
{
	Log::Exception("Failed to create window surface.", glfwCreateWindowSurface(Graphic::Core::Instance::_vkInstance, _glfwWindow, nullptr, &_vkSurface));
}
void Graphic::Core::Window::_CreateSwapchain()
{
    VkSurfaceCapabilitiesKHR capabilities{};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(Device::_vkPhysicalDevice, _vkSurface, &capabilities);
    {
        uint32_t formatCount = 0;
        vkGetPhysicalDeviceSurfaceFormatsKHR(Device::_vkPhysicalDevice, _vkSurface, &formatCount, nullptr);
        std::vector<VkSurfaceFormatKHR> formats = std::vector<VkSurfaceFormatKHR>(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(Device::_vkPhysicalDevice, _vkSurface, &formatCount, formats.data());
    }
    uint32_t imageCount = capabilities.minImageCount + 1;
    if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount) {
        imageCount = capabilities.maxImageCount;
    }

    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
    {
        _extent = capabilities.currentExtent;
    }
    else
    {
        int width, height;
        glfwGetFramebufferSize(_glfwWindow, &width, &height);

        VkExtent2D actualExtent = {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };

        actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        _extent = actualExtent;
    }

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = _vkSurface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = _imageFormat;
    createInfo.imageColorSpace = _imageColorSpace;
    createInfo.imageExtent = _extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = _imageUsage;
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.preTransform = capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = _presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    Log::Message("Failed to create swap chain.", vkCreateSwapchainKHR(Device::_vkDevice, &createInfo, nullptr, &_vkSwapchain));

}

void Graphic::Core::Window::_CreateSwapchainImages()
{
    uint32_t imageCount = 0;
    vkGetSwapchainImagesKHR(Device::_vkDevice, _vkSwapchain, &imageCount, nullptr);
    std::vector<VkImage> scImages = std::vector<VkImage>(imageCount);
    _swapchainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(Device::_vkDevice, _vkSwapchain, &imageCount, scImages.data());
    for (uint32_t i = 0; i < imageCount; i++)
    {
        _swapchainImages[i] = new Graphic::Instance::SwapchainImage(scImages[i], _imageFormat, _extent, _imageUsage);
    }
}
