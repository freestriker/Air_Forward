#pragma once
#include <vulkan/vulkan_core.h>
#include <vector>
#include <string>
namespace Graphic
{
	namespace Core
	{
		class Window;
		class Device;
		class Instance
		{
			friend class Graphic::Core::Window;
			friend class Graphic::Core::Device;
		public:
			class InstanceCreator
			{
				friend class Graphic::Core::Instance;
			public:
				std::string applicationName;
				uint32_t applicationVersion;
				std::string engineName;
				uint32_t engineVersion;
				uint32_t apiVersion;
#ifdef _USE_GRAPHIC_DEBUG
				VkDebugUtilsMessageSeverityFlagsEXT messageSeverity;
				VkDebugUtilsMessageTypeFlagsEXT messageType;
				PFN_vkDebugUtilsMessengerCallbackEXT debugCallback;
#endif
			private:
				std::vector<VkExtensionProperties> _availableExtensions;
				std::vector<std::string> _desiredExtensions;
#ifdef _USE_GRAPHIC_DEBUG
				std::vector<VkLayerProperties> _availableLayers;
				std::vector<std::string> _desiredLayers;
#endif

			public:
				InstanceCreator();
				~InstanceCreator();
				void AddExtension(std::string extensionName);
#ifdef _USE_GRAPHIC_DEBUG
				void AddLayer(std::string layerName);
#endif
			};

			static void Create(InstanceCreator& creator);
			static inline VkInstance VkInstance_();
		private:
			static VkInstance _vkInstance;
			static void _AddWindowExtension(InstanceCreator& creator);
#ifdef _USE_GRAPHIC_DEBUG
			static VkDebugUtilsMessengerEXT _debugMessenger;
			static void _AddDebugExtension(InstanceCreator& creator);
			static void _CreateDebugMessenger(InstanceCreator& creator);
			static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
#endif
		};
	}
}

VkInstance Graphic::Core::Instance::VkInstance_()
{
	return _vkInstance;
}

