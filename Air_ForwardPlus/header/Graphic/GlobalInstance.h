#pragma once
#include <vulkan/vulkan_core.h>
#include <GLFW/glfw3.h>
#define GLFW_INCLUDE_VULKAN
#include <map>
#include <string>
#include <vector>

namespace Graphic
{
	class GraphicQueue
	{
	public:
		std::string name;
		uint32_t queueFamilyIndex;
		VkQueue queue;
		GraphicQueue(std::string& name, uint32_t queueFamilyIndex, VkQueue queue) :
			name(name), queueFamilyIndex(queueFamilyIndex), queue(queue)
		{

		}
		GraphicQueue()
			: name()
			, queueFamilyIndex()
			, queue(VK_NULL_HANDLE)
		{

		}
		~GraphicQueue()
		{

		}
	};

	class VulkanDeviceCreator;
	class VulkanInstanceCreator;
	class GlfwWindowCreator;
	class RenderPassCreator;
	class GlobalInstance final
	{
		friend class VulkanInstanceCreator;
	public:
		static VkInstance instance;
		static GLFWwindow* window;
		static VkSurfaceKHR surface;
		static VkPhysicalDevice physicalDevice;
		static VkDevice device;
		static std::map<std::string, GraphicQueue> queues;
		static VkSwapchainKHR windowSwapchain;
		static std::vector<VkImage> windowSwapchainImages;
		static std::vector<VkImageView> windowSwapchainImageViews;
		static std::vector<VkSemaphore> windowImageAvailableSemaphores;
		static std::vector<VkSemaphore> renderImageFinishedSemaphores;
		static std::vector<VkFence> frameInFlightFences;
		static std::map<std::string, VkRenderPass> renderpasss;


	private:
#ifdef _USE_GRAPHIC_DEBUG
		static VkDebugUtilsMessengerEXT _debugMessenger;
#endif

	public:
		static void CreateGlfwWindow(GlfwWindowCreator* creator);
		static void CreateVulkanInstance(VulkanInstanceCreator* creator);
		static void CreateVulkanDevice(VulkanDeviceCreator* creator);

		static void CreateRenderPass(RenderPassCreator* creator);
	private:
		GlobalInstance();
		~GlobalInstance();
		static void AddDeviceWindowParameter(VulkanDeviceCreator* creator);
		static void AddInstanceWindowparameter(VulkanInstanceCreator* creator);
		static void CreateWindowSurface();
		static void CreateWindowSwapchain();
		static void CreateWindowSwapchainImages();
#ifdef _USE_GRAPHIC_DEBUG
		static void AddInstanceDebugExtension(VulkanInstanceCreator* creator);
		static void CreateDebugMessenger(VulkanInstanceCreator* creator);
		static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
#endif
	};
}
