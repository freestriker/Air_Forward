#pragma once
#include "Graphic/Core/Device.h"
#include "Graphic/Core/Instance.h"
#include "Graphic/Core/Window.h"
#include <Utils/Log.h>
using namespace Utils;
#include "Graphic/Manager/MemoryManager.h"
#include "Graphic/Manager/RenderPassManager.h"
#include "Graphic/Manager/DescriptorSetManager.h"
#include "Graphic/Manager/FrameBufferManager.h"

VkPhysicalDevice Graphic::Core::Device::_vkPhysicalDevice = VK_NULL_HANDLE;
VkDevice Graphic::Core::Device::_vkDevice = VK_NULL_HANDLE;
std::map<std::string, Graphic::Core::Device::Queue*> Graphic::Core::Device::_queues = std::map<std::string, Graphic::Core::Device::Queue*>();

Graphic::Manager::MemoryManager* Graphic::Core::Device::_memoryManager = nullptr;
Graphic::Manager::RenderPassManager* Graphic::Core::Device::_renderPassManager = nullptr;
Graphic::Manager::DescriptorSetManager* Graphic::Core::Device::_descriptorSetManager = nullptr;
Graphic::Manager::FrameBufferManager* Graphic::Core::Device::_frameBufferManager = nullptr;


Graphic::Core::Device::DeviceCreator::DeviceCreator()
    : desiredPhysicalDeviceType(VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
    , _desiredPhysicalDeviceFeatures()
    , _desiredDeviceExtensions()
    , _desiredQueues()
#ifdef _USE_GRAPHIC_DEBUG
    , _desiredDeviceLayers()
#endif
{

}

Graphic::Core::Device::DeviceCreator::~DeviceCreator()
{

}

void Graphic::Core::Device::DeviceCreator::AddExtension(std::string extensionName)
{
    _desiredDeviceExtensions.emplace_back(extensionName);
}

void Graphic::Core::Device::DeviceCreator::SetFeature(std::function<void(VkPhysicalDeviceFeatures&)> const& func)
{
    func(_desiredPhysicalDeviceFeatures);
}

#ifdef _USE_GRAPHIC_DEBUG
void Graphic::Core::Device::DeviceCreator::AddLayer(std::string layerName)
{
    _desiredDeviceLayers.emplace_back(layerName);
}
#endif

void Graphic::Core::Device::DeviceCreator::AddQueue(std::string name, VkQueueFlags flag, float prioritie)
{
    _desiredQueues.emplace_back(name, flag, prioritie);
}

void Graphic::Core::Device::Create(Graphic::Core::Device::DeviceCreator& creator)
{
    _AddWindowExtension(creator);

    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(Core::Instance::_vkInstance, &deviceCount, nullptr);
    Log::Exception("Failed to find GPUs with vulkan support.", deviceCount == 0);
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(Core::Instance::_vkInstance, &deviceCount, devices.data());

    for (const auto& device : devices)
    {
        VkPhysicalDeviceProperties deviceProperties = VkPhysicalDeviceProperties();
        vkGetPhysicalDeviceProperties(device, &deviceProperties);

        if ((deviceProperties.deviceType & creator.desiredPhysicalDeviceType) != creator.desiredPhysicalDeviceType) continue;

#ifdef _USE_GRAPHIC_DEBUG
        uint32_t layerCount = 0;
        uint32_t foundLayerCount = 0;
        vkEnumerateDeviceLayerProperties(device, &layerCount, nullptr);
        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateDeviceLayerProperties(device, &layerCount, availableLayers.data());
        for (const auto& desiredLayer : creator._desiredDeviceLayers)
        {
            for (const auto& availableLayer : availableLayers)
            {
                if (strcmp(availableLayer.layerName, desiredLayer.c_str()) == 0)
                {
                    ++foundLayerCount;
                    break;
                }
            }
        }
        if (foundLayerCount != creator._desiredDeviceLayers.size()) continue;
#endif

        uint32_t extensionCount = 0;
        uint32_t foundExtensionCount = 0;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());
        for (const auto& desiredExtension : creator._desiredDeviceExtensions)
        {
            for (const auto& availableExtension : availableExtensions)
            {
                if (strcmp(availableExtension.extensionName, desiredExtension.c_str()) == 0)
                {
                    ++foundExtensionCount;
                    break;
                }
            }
        }
        if (foundExtensionCount != creator._desiredDeviceExtensions.size()) continue;

        uint32_t queueFamilyCount = 0;
        uint32_t foundQueueCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());
        std::vector<uint32_t> usedCounts = std::vector<uint32_t>(queueFamilyCount, 0);
        std::vector<uint32_t> usedIndexs = std::vector<uint32_t>(creator._desiredQueues.size(), -1);
        for (uint32_t i = 0; i < creator._desiredQueues.size(); i++)
        {
            const auto& desiredQueue = creator._desiredQueues[i];

            if (Core::Window::_glfwWindow && strcmp(desiredQueue.name.c_str(), "PresentQueue") == 0)
            {
                for (uint32_t j = 0; j < queueFamilies.size(); j++)
                {
                    const auto& queueFamilie = queueFamilies[j];

                    VkBool32 presentSupport = VK_FALSE;
                    vkGetPhysicalDeviceSurfaceSupportKHR(device, j, Core::Window::_vkSurface, &presentSupport);
                    if (presentSupport && queueFamilie.queueCount - usedCounts[j] > 1)
                    {
                        usedIndexs[i] = j;
                        usedCounts[j] += 1;
                        ++foundQueueCount;
                        break;
                    }
                }
            }
            else
            {
                for (uint32_t j = 0; j < queueFamilies.size(); j++)
                {
                    const auto& queueFamilie = queueFamilies[j];
                    if ((desiredQueue.flag & queueFamilie.queueFlags) == desiredQueue.flag && queueFamilie.queueCount - usedCounts[j] > 1)
                    {
                        usedIndexs[i] = j;
                        usedCounts[j] += 1;
                        ++foundQueueCount;
                        break;
                    }
                }
            }
        }
        if (foundQueueCount != creator._desiredQueues.size()) continue;
        std::map<uint32_t, std::vector<float>> queuePrioritieMap = std::map<uint32_t, std::vector<float>>();

        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        {
            std::map<uint32_t, VkDeviceQueueCreateInfo> queueCreateInfoMap = std::map<uint32_t, VkDeviceQueueCreateInfo>();
            for (uint32_t i = 0; i < usedIndexs.size(); i++)
            {
                const uint32_t& queueIndex = usedIndexs[i];
                if (queueCreateInfoMap.count(queueIndex))
                {
                    queueCreateInfoMap[queueIndex].queueCount += 1;
                    queuePrioritieMap[queueIndex].push_back(creator._desiredQueues[i].prioritie);
                }
                else
                {
                    VkDeviceQueueCreateInfo queueCreateInfo{};
                    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
                    queueCreateInfo.queueFamilyIndex = queueIndex;
                    queueCreateInfo.queueCount = 1;

                    queueCreateInfoMap.insert(std::pair <uint32_t, VkDeviceQueueCreateInfo>(queueIndex, queueCreateInfo));
                    queuePrioritieMap.insert(std::pair < uint32_t, std::vector<float>>(queueIndex, { creator._desiredQueues[i].prioritie }));
                }
            }
            queueCreateInfos.resize(queueCreateInfoMap.size());
            uint32_t i = 0;
            for (const auto& info : queueCreateInfoMap)
            {
                queueCreateInfos[i] = info.second;
                queueCreateInfos[i].pQueuePriorities = queuePrioritieMap[info.first].data();
            }
        }

        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.pNext = nullptr;

        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos = queueCreateInfos.data();

        createInfo.pEnabledFeatures = &creator._desiredPhysicalDeviceFeatures;

        std::vector<const char*> enabledExtensionNames = std::vector<const char*>(creator._desiredDeviceExtensions.size());
        for (uint32_t i = 0; i < enabledExtensionNames.size(); i++)
        {
            enabledExtensionNames[i] = creator._desiredDeviceExtensions[i].c_str();
        }

        createInfo.enabledExtensionCount = static_cast<uint32_t>(enabledExtensionNames.size());
        createInfo.ppEnabledExtensionNames = enabledExtensionNames.data();

#ifdef _USE_GRAPHIC_DEBUG
        std::vector<const char*> enabledLayerNames = std::vector<const char*>(creator._desiredDeviceLayers.size());
        for (uint32_t i = 0; i < enabledLayerNames.size(); i++)
        {
            enabledLayerNames[i] = creator._desiredDeviceLayers[i].c_str();
        }
        createInfo.enabledLayerCount = static_cast<uint32_t>(enabledLayerNames.size());
        createInfo.ppEnabledLayerNames = enabledLayerNames.data();
#endif

        _vkPhysicalDevice = device;

        Log::Exception("Failed to create logical device.", vkCreateDevice(_vkPhysicalDevice, &createInfo, nullptr, &_vkDevice));

        usedCounts = std::vector<uint32_t>(queueFamilyCount, 0);
        for (uint32_t i = 0; i < usedIndexs.size(); i++)
        {
            const uint32_t& queueIndex = usedIndexs[i];
            std::string name = std::string(creator._desiredQueues[i].name);
            Queue* gq = new Queue(name, queueIndex, VK_NULL_HANDLE);
            vkGetDeviceQueue(_vkDevice, queueIndex, usedCounts[queueIndex]++, &(gq->queue));
            _queues.emplace(name, gq);
        }

        _CreateManager(creator);

        Graphic::Core::Window::_CreateSwapchain();
        Graphic::Core::Window::_CreateSwapchainImages();
        return;
    }

    Log::Exception("Failed to find suitable device.");

}

void Graphic::Core::Device::_AddWindowExtension(Graphic::Core::Device::DeviceCreator& creator)
{
    creator.AddExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    creator.AddQueue("PresentQueue", VK_QUEUE_FLAG_BITS_MAX_ENUM, 1.0f);
#ifdef _USE_GRAPHIC_DEBUG
    creator.AddLayer("VK_LAYER_KHRONOS_validation");
    creator.AddLayer("VK_LAYER_RENDERDOC_Capture");
#endif
}

void Graphic::Core::Device::_CreateManager(Graphic::Core::Device::DeviceCreator& creator)
{
    _memoryManager = new Manager::MemoryManager(32 * 1024 * 1024);
    _renderPassManager = new Manager::RenderPassManager();
    _descriptorSetManager = new Manager::DescriptorSetManager();
    _frameBufferManager = new Manager::FrameBufferManager();
}
