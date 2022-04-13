#pragma once
#include <vector>
#include <vulkan/vulkan_core.h>

namespace Graphic
{
	class VulkanInstanceCreator final
	{
		friend class GlobalInstance;
	public:
		const char* applicationName;
		uint32_t applicationVersion;
		const char* engineName;
		uint32_t engineVersion;
		uint32_t apiVersion;
#ifdef _USE_GRAPHIC_DEBUG
		VkDebugUtilsMessageSeverityFlagsEXT messageSeverity;
		VkDebugUtilsMessageTypeFlagsEXT messageType;
		PFN_vkDebugUtilsMessengerCallbackEXT debugCallback;
#endif
	private:
		std::vector<VkExtensionProperties> _availableExtensions;
		std::vector<char const*> _desiredExtensions;
#ifdef _USE_GRAPHIC_DEBUG
		std::vector<VkLayerProperties> _availableLayers;
		std::vector<char const*> _desiredLayers;
#endif

	public:
		VulkanInstanceCreator();
		~VulkanInstanceCreator();
		void AddExtension(char const* extensionName);
#ifdef _USE_GRAPHIC_DEBUG
		void AddLayer(char const* layerName);
#endif
	};
}