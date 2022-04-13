#include "Graphic/Creator/VulkanInstanceCreator.h"
#include <stdexcept>
#include "Graphic/GlobalInstance.h"

Graphic::VulkanInstanceCreator::VulkanInstanceCreator()
	: applicationName("Vulkan Application")
	, applicationVersion(VK_MAKE_VERSION(1, 0, 0))
	, engineName("No Engine")
	, engineVersion(VK_MAKE_VERSION(1, 0, 0))
	, apiVersion(VK_API_VERSION_1_0)
	, messageSeverity(/*VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | */VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
	, messageType(VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT)
	, debugCallback(Graphic::GlobalInstance::DebugCallback)
{
	uint32_t extensionCount;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
	_availableExtensions.resize(extensionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, _availableExtensions.data());

#ifdef _USE_GRAPHIC_DEBUG
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
	_availableLayers.resize(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, _availableLayers.data());
#endif
}

Graphic::VulkanInstanceCreator::~VulkanInstanceCreator()
{

}

void Graphic::VulkanInstanceCreator::AddExtension(char const* extensionName)
{
	for (const auto& availableExtension : _availableExtensions)
	{
		if (strcmp(availableExtension.extensionName, extensionName) == 0)
		{
			_desiredExtensions.push_back(extensionName);
			return;
		}
	}
	std::string err = "Do not exist extension named ";
	err += extensionName;
	err += ".";
	throw std::runtime_error(err);
}

#ifdef _USE_GRAPHIC_DEBUG
void Graphic::VulkanInstanceCreator::AddLayer(char const* layerName)
{
	for (const auto& availableLayer : _availableLayers)
	{
		if (strcmp(availableLayer.layerName, layerName) == 0)
		{
			_desiredLayers.push_back(layerName);
			return;
		}
	}
	std::string err = "Do not exist layer named ";
	err += layerName;
	err += ".";
	throw std::runtime_error(err);
}
#endif
