#pragma once
#include <vulkan/vulkan_core.h>
#include <string>
#include <GLFW/glfw3.h>
#include <vector>
#define GLFW_INCLUDE_VULKAN

namespace Graphic
{
	namespace Instance
	{
		class SwapchainImage;
	}
	namespace Core
	{
		class Instance;
		class Device;
		class Window
		{
			friend class Graphic::Core::Instance;
			friend class Graphic::Core::Device;
		private:
			static VkFormat _imageFormat;
			static VkColorSpaceKHR _imageColorSpace;
			static VkPresentModeKHR _presentMode;
			static VkImageUsageFlags _imageUsage;
			static VkExtent2D _extent;
			static uint32_t _maxFrameInFlightCount;
			static GLFWwindow* _glfwWindow;
			static VkSurfaceKHR _vkSurface;
			static VkSwapchainKHR _vkSwapchain;
			static std::vector<Graphic::Instance::SwapchainImage*> _swapchainImages;
			static void _CreateSurface();
			static void _CreateSwapchain();
			static void _CreateSwapchainImages();

		public:
			class WindowCreator final
			{
			public:
				uint32_t width;
				uint32_t height;
				std::string title;
				VkFormat windowImageFormat;
				VkColorSpaceKHR windowImageColorSpace;
				VkPresentModeKHR windowPresentMode;
				VkImageUsageFlags windowImageUsage;
				uint32_t maxFrameInFlightCount;
			public:
				WindowCreator();
				~WindowCreator();
			};
			static void Create(WindowCreator& creator);

			static inline VkFormat VkFormat_();
			static inline VkExtent2D VkExtent2D_();
			static inline GLFWwindow* GLFWwindow_();
			static inline VkSurfaceKHR VkSurfaceKHR_();
			static inline VkSwapchainKHR VkSwapchainKHR_();
			static inline Graphic::Instance::SwapchainImage* SwapchainImage_(uint32_t index);
		};
	}
}

inline VkFormat Graphic::Core::Window::VkFormat_()
{
	return _imageFormat;
}
inline VkExtent2D Graphic::Core::Window::VkExtent2D_()
{
	return _extent;
}
inline GLFWwindow* Graphic::Core::Window::GLFWwindow_()
{
	return _glfwWindow;
}
inline VkSurfaceKHR Graphic::Core::Window::VkSurfaceKHR_()
{
	return _vkSurface;
}
inline VkSwapchainKHR Graphic::Core::Window::VkSwapchainKHR_()
{
	return _vkSwapchain;
}
inline Graphic::Instance::SwapchainImage* Graphic::Core::Window::SwapchainImage_(uint32_t index)
{
	return _swapchainImages[index];
}
