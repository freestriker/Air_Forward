#pragma once
#include <vulkan/vulkan_core.h>
#include <vector>
#include <functional>
namespace Graphic
{
	class VulkanDeviceCreator
	{
		friend class GlobalInstance;
	private:
		class DesiredQueue final
		{
			friend class GlobalInstance;
		public:
			const char* name;
			VkQueueFlags flag;
			float prioritie;
			DesiredQueue(const char* name, VkQueueFlags flag, float prioritie) : name(name), flag(flag), prioritie(prioritie)
			{

			}
			~DesiredQueue()
			{

			}
		};
		VkPhysicalDeviceFeatures _desiredPhysicalDeviceFeatures;
		std::vector<const char*> _desiredDeviceExtensions;
		std::vector<DesiredQueue> _desiredQueues;
#ifdef _USE_GRAPHIC_DEBUG
		std::vector<const char*> _desiredDeviceLayers;
#endif
	public:
		VkPhysicalDeviceType desiredPhysicalDeviceType;

	public:
		VulkanDeviceCreator();
		~VulkanDeviceCreator();
		void AddDeviceExtension(const char* extensionName);
		void SetDeviceFeature(std::function<void(VkPhysicalDeviceFeatures&)> const& func);
#ifdef _USE_GRAPHIC_DEBUG
		void AddDeviceLayer(const char* layerName);
#endif
		void AddQueue(const char* name, VkQueueFlags flag, float prioritie);
	};
}
