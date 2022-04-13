#include "Graphic/Creator/VulkanDeviceCreator.h"

Graphic::VulkanDeviceCreator::VulkanDeviceCreator()
	: desiredPhysicalDeviceType(VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
    , _desiredPhysicalDeviceFeatures()
    , _desiredDeviceExtensions()
    , _desiredQueues()
#ifdef _USE_GRAPHIC_DEBUG
    , _desiredDeviceLayers()
#endif
{
}

Graphic::VulkanDeviceCreator::~VulkanDeviceCreator()
{
}

void Graphic::VulkanDeviceCreator::AddDeviceExtension(const char* extensionName)
{
    _desiredDeviceExtensions.emplace_back(extensionName);
}

void Graphic::VulkanDeviceCreator::SetDeviceFeature(std::function<void(VkPhysicalDeviceFeatures&)> const& func)
{
    func(_desiredPhysicalDeviceFeatures);
}

#ifdef _USE_GRAPHIC_DEBUG
void Graphic::VulkanDeviceCreator::AddDeviceLayer(const char* layerName)
{
    _desiredDeviceLayers.emplace_back(layerName);
}
#endif

void Graphic::VulkanDeviceCreator::AddQueue(const char* name, VkQueueFlags flag, float prioritie)
{
    _desiredQueues.emplace_back(name, flag, prioritie);
}
