#include "Graphic/Creator/VulkanInstanceCreator.h"
#include <stdexcept>
#include "Graphic/GlobalInstance.h"
#include "utils/DebugUtils.h"
Graphic::VulkanInstanceCreator::VulkanInstanceCreator()
	: applicationName("Vulkan Application")
	, applicationVersion(VK_MAKE_VERSION(1, 0, 0))
	, engineName("No Engine")
	, engineVersion(VK_MAKE_VERSION(1, 0, 0))
	, apiVersion(VK_API_VERSION_1_0)
#ifdef _USE_GRAPHIC_DEBUG
	, messageSeverity(/*VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | */VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
	, messageType(VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT)
	, debugCallback(Graphic::GlobalInstance::DebugCallback)
#endif
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
	Debug::Log("Do not exist extension named " + std::string(extensionName) + ".");
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
	Debug::Log("Do not exist layer named " + std::string(layerName) + ".");
}
#endif
