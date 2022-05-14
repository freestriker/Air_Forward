#include "Graphic/Core/Instance.h"
#include "Graphic/Core/Window.h"
#include "utils/Log.h"
#include <GLFW/glfw3.h>
#define GLFW_INCLUDE_VULKAN

VkInstance Graphic::Core::Instance::_vkInstance = VK_NULL_HANDLE;
VkDebugUtilsMessengerEXT Graphic::Core::Instance::_debugMessenger = VK_NULL_HANDLE;

Graphic::Core::Instance::Creator::Creator()
	: applicationName("Vulkan Application")
	, applicationVersion(VK_MAKE_VERSION(1, 0, 0))
	, engineName("No Engine")
	, engineVersion(VK_MAKE_VERSION(1, 0, 0))
	, apiVersion(VK_API_VERSION_1_0)
#ifdef _USE_GRAPHIC_DEBUG
	, messageSeverity(VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
	, messageType(VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT)
	, debugCallback(Graphic::Core::Instance::DebugCallback)
#endif

{
}

Graphic::Core::Instance::Creator::~Creator()
{
}

#ifdef _USE_GRAPHIC_DEBUG
void Graphic::Core::Instance::Creator::AddLayer(std::string layerName)
{
	for (const auto& availableLayer : _availableLayers)
	{
		if (strcmp(availableLayer.layerName, layerName.c_str()) == 0)
		{
			_desiredLayers.push_back(layerName);
			return;
		}
	}
	Log::Exception("Do not exist layer named " + std::string(layerName) + ".");
}
#endif

void Graphic::Core::Instance::Creator::AddExtension(std::string extensionName)
{
	for (const auto& availableExtension : _availableExtensions)
	{
		if (strcmp(availableExtension.extensionName, extensionName.c_str()) == 0)
		{
			_desiredExtensions.push_back(extensionName);
			return;
		}
	}
	Log::Exception("Do not exist extension named " + std::string(extensionName) + ".");
}

void Graphic::Core::Instance::Create(Creator& creator)
{
#ifdef _USE_GRAPHIC_DEBUG
	_AddDebugExtension(creator);
#endif
	_AddWindowExtension(creator);

	VkApplicationInfo applicationInfo{};
	applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	applicationInfo.pApplicationName = creator.applicationName.c_str();
	applicationInfo.applicationVersion = creator.applicationVersion;
	applicationInfo.pEngineName = creator.engineName.c_str();
	applicationInfo.engineVersion = creator.engineVersion;
	applicationInfo.apiVersion = creator.apiVersion;

	VkInstanceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &applicationInfo;

	std::vector<const char*> enabledExtensionNames = std::vector<const char*>(creator._desiredExtensions.size());
	for (uint32_t i = 0; i < enabledExtensionNames.size(); i++)
	{
		enabledExtensionNames[i] = creator._desiredExtensions[i].c_str();
	}
	createInfo.enabledExtensionCount = static_cast<uint32_t>(enabledExtensionNames.size());
	createInfo.ppEnabledExtensionNames = enabledExtensionNames.data();

#ifdef _USE_GRAPHIC_DEBUG

	std::vector<const char*> enabledLayerNames = std::vector<const char*>(creator._desiredLayers.size());
	for (uint32_t i = 0; i < enabledLayerNames.size(); i++)
	{
		enabledLayerNames[i] = creator._desiredLayers[i].c_str();
	}
	createInfo.enabledLayerCount = static_cast<uint32_t>(enabledLayerNames.size());
	createInfo.ppEnabledLayerNames = enabledLayerNames.data();

#endif

	Log::Exception("Failed to create instance.", vkCreateInstance(&createInfo, nullptr, &_vkInstance));

#ifdef _USE_GRAPHIC_DEBUG
	_CreateDebugMessenger(creator);
#endif

	Graphic::Core::Window::_CreateSurface();
}

void Graphic::Core::Instance::_AddWindowExtension(Creator& creator)
{
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensionNames = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
	std::vector<const char*> glfwExtensions(glfwExtensionNames, glfwExtensionNames + glfwExtensionCount);
	for (const auto& windowExtension : glfwExtensions)
	{
		creator.AddExtension(windowExtension);
	}
}

#ifdef _USE_GRAPHIC_DEBUG
VKAPI_ATTR VkBool32 VKAPI_CALL Graphic::Core::Instance::DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
	Log::Message(pCallbackData->pMessage);
	return VK_FALSE;
}

void Graphic::Core::Instance::_AddDebugExtension(Creator& creator)
{
	creator.AddLayer("VK_LAYER_KHRONOS_validation");
	creator.AddLayer("VK_LAYER_RENDERDOC_Capture");
	creator.AddExtension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
}

void Graphic::Core::Instance::_CreateDebugMessenger(Creator& creator)
{
	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
	debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	debugCreateInfo.messageSeverity = creator.messageSeverity;
	debugCreateInfo.messageType = creator.messageType;
	debugCreateInfo.pfnUserCallback = creator.debugCallback;

	VkResult result;
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(_vkInstance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr)
	{
		result = func(_vkInstance, &debugCreateInfo, nullptr, &_debugMessenger);
	}
	else
	{
		result = VK_ERROR_EXTENSION_NOT_PRESENT;
	}

	Log::Exception("Failed to set up debug messenger.", result);
}
#endif