#include "Graphic/GlobalInstance.h"
#include <iostream>
#include "Graphic/Creator/VulkanInstanceCreator.h"
#include "Graphic/Creator/GlfwWindowCreator.h"
#include "Graphic/Creator/VulkanDeviceCreator.h"
#include <algorithm>
#include <Graphic/GlobalSetting.h>
#include "Graphic/MemoryManager.h"
#include "Graphic/RenderPassUtils.h"
#include "Graphic/DescriptorSetUtils.h"
#include "Graphic/FrameBufferUtils.h"

VkInstance Graphic::GlobalInstance::instance(VK_NULL_HANDLE);
GLFWwindow* Graphic::GlobalInstance::window(nullptr);
#ifdef _USE_GRAPHIC_DEBUG
VkDebugUtilsMessengerEXT Graphic::GlobalInstance::_debugMessenger(VK_NULL_HANDLE);
#endif
VkSurfaceKHR Graphic::GlobalInstance::surface(VK_NULL_HANDLE);
VkPhysicalDevice Graphic::GlobalInstance::physicalDevice(VK_NULL_HANDLE);
VkDevice Graphic::GlobalInstance::device(VK_NULL_HANDLE);
std::map<std::string, Graphic::Queue*> Graphic::GlobalInstance::queues = std::map<std::string, Graphic::Queue*>();
VkSwapchainKHR Graphic::GlobalInstance::windowSwapchain(VK_NULL_HANDLE);
std::vector<VkImage> Graphic::GlobalInstance::windowSwapchainImages({});
std::vector<VkImageView> Graphic::GlobalInstance::windowSwapchainImageViews({});
std::vector<VkSemaphore> Graphic::GlobalInstance::windowImageAvailableSemaphores({});
std::vector<VkSemaphore> Graphic::GlobalInstance::renderImageFinishedSemaphores({});
std::vector<VkFence> Graphic::GlobalInstance::frameInFlightFences({});
Graphic::MemoryManager* Graphic::GlobalInstance::memoryManager = nullptr;
Graphic::Render::RenderPassManager* const Graphic::GlobalInstance::renderPassManager = new Graphic::Render::RenderPassManager();
Graphic::Manager::DescriptorSetManager* const Graphic::GlobalInstance::descriptorSetManager = new Graphic::Manager::DescriptorSetManager();
Graphic::Manager::FrameBufferManager* const Graphic::GlobalInstance::frameBufferManager = new Graphic::Manager::FrameBufferManager();

Graphic::GlobalInstance::GlobalInstance()
{
}
Graphic::GlobalInstance::~GlobalInstance()
{
}

void Graphic::GlobalInstance::AddDeviceWindowParameter(VulkanDeviceCreator* creator)
{
    creator->AddDeviceExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    creator->AddQueue("PresentQueue", VK_QUEUE_FLAG_BITS_MAX_ENUM, 1.0f);
#ifdef _USE_GRAPHIC_DEBUG
    creator->AddDeviceLayer("VK_LAYER_KHRONOS_validation");
#endif
}

#ifdef _USE_GRAPHIC_DEBUG
void Graphic::GlobalInstance::AddInstanceDebugExtension(VulkanInstanceCreator* creator)
{
	creator->AddLayer("VK_LAYER_KHRONOS_validation");
	creator->AddExtension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
}
void Graphic::GlobalInstance::CreateDebugMessenger(VulkanInstanceCreator* creator)
{
	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
	debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	debugCreateInfo.messageSeverity = creator->messageSeverity;
	debugCreateInfo.messageType = creator->messageType;
	debugCreateInfo.pfnUserCallback = creator->debugCallback;

	VkResult result;
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr)
	{
		result = func(instance, &debugCreateInfo, nullptr, &_debugMessenger);
	}
	else
	{
		result = VK_ERROR_EXTENSION_NOT_PRESENT;
	}

	if (result != VK_SUCCESS) {
		std::string err = "Failed to set up debug messenger, errcode ";
		err += std::to_string(result);
		err += ".";
		throw std::runtime_error(err);
	}
}

VKAPI_ATTR VkBool32 VKAPI_CALL Graphic::GlobalInstance::DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
    return VK_FALSE;
}
#endif

void Graphic::GlobalInstance::CreateGlfwWindow(GlfwWindowCreator* creator)
{
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	window = glfwCreateWindow(creator->width, creator->height, creator->title, nullptr, nullptr);

    Graphic::GlobalSetting::windowImageColorSpace = creator->windowImageColorSpace;
    Graphic::GlobalSetting::windowImageFormat = creator->windowImageFormat;
    Graphic::GlobalSetting::windowImageUsage = creator->windowImageUsage;
    Graphic::GlobalSetting::windowPresentMode = creator->windowPresentMode;
    Graphic::GlobalSetting::maxFrameInFlightCount = creator->maxFrameInFlightCount;
}
void Graphic::GlobalInstance::AddInstanceWindowparameter(VulkanInstanceCreator* creator)
{
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensionNames = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
	std::vector<const char*> glfwExtensions(glfwExtensionNames, glfwExtensionNames + glfwExtensionCount);
	for (const auto& windowExtension : glfwExtensions)
	{
		creator->AddExtension(windowExtension);
	}
}

void Graphic::GlobalInstance::CreateWindowSurface()
{
    VkResult surfaceResult = glfwCreateWindowSurface(instance, window, nullptr, &surface);
    if (surfaceResult != VK_SUCCESS) {
        std::string err = "Failed to create window surface, errcode: ";
        err += surfaceResult;
        err += ".";
        throw std::runtime_error(err);
    }
}

void Graphic::GlobalInstance::CreateWindowSwapchain()
{
    VkSurfaceCapabilitiesKHR capabilities{};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &capabilities);

    uint32_t imageCount = capabilities.minImageCount + 1;
    if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount) {
        imageCount = capabilities.maxImageCount;
    }

    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
    {
        GlobalSetting::windowExtent = capabilities.currentExtent;
    }
    else
    {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        VkExtent2D actualExtent = {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };

        actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        GlobalSetting::windowExtent = actualExtent;
    }

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = GlobalSetting::windowImageFormat;
    createInfo.imageColorSpace = GlobalSetting::windowImageColorSpace;
    createInfo.imageExtent = GlobalSetting::windowExtent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = GlobalSetting::windowImageUsage;
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.preTransform = capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = GlobalSetting::windowPresentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    VkResult swapChainResult = vkCreateSwapchainKHR(device, &createInfo, nullptr, &windowSwapchain);
    if (swapChainResult != VK_SUCCESS) {
        std::string err = "Failed to create swap chain, errcode: ";
        err += swapChainResult;
        err += ".";
        throw std::runtime_error(err);
    }

}

void Graphic::GlobalInstance::CreateWindowSwapchainImages()
{
    
    uint32_t imageCount = 0;
    vkGetSwapchainImagesKHR(device, windowSwapchain, &imageCount, nullptr);
    windowSwapchainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(device, windowSwapchain, &imageCount, windowSwapchainImages.data());

    //windowSwapchainImageViews.resize(imageCount);
    //for (size_t i = 0; i < imageCount; i++)
    //{
    //    VkImageViewCreateInfo createInfo{};
    //    createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    //    createInfo.image = windowSwapchainImages[i];
    //    createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    //    createInfo.format = GlobalSetting::windowImageFormat;
    //    createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    //    createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    //    createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    //    createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    //    createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    //    createInfo.subresourceRange.baseMipLevel = 0;
    //    createInfo.subresourceRange.levelCount = 1;
    //    createInfo.subresourceRange.baseArrayLayer = 0;
    //    createInfo.subresourceRange.layerCount = 1;

    //    VkResult imageViewResult = vkCreateImageView(device, &createInfo, nullptr, &windowSwapchainImageViews[i]);
    //    if (imageViewResult != VK_SUCCESS)
    //    {
    //        std::string err = "Failed to create image view, errcode: ";
    //        err += imageViewResult;
    //        err += ".";
    //        throw std::runtime_error(err);
    //    }
    //}

    //windowImageAvailableSemaphores.resize(GlobalSetting::maxFrameInFlightCount);
    //renderImageFinishedSemaphores.resize(GlobalSetting::maxFrameInFlightCount);
    //frameInFlightFences.resize(GlobalSetting::maxFrameInFlightCount);

    //VkSemaphoreCreateInfo semaphoreInfo{};
    //semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    //VkFenceCreateInfo fenceInfo{};
    //fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    //fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    //for (size_t i = 0; i < GlobalSetting::maxFrameInFlightCount; i++) {
    //    if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &windowImageAvailableSemaphores[i]) != VK_SUCCESS ||
    //        vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderImageFinishedSemaphores[i]) != VK_SUCCESS ||
    //        vkCreateFence(device, &fenceInfo, nullptr, &frameInFlightFences[i]) != VK_SUCCESS) {
    //        throw std::runtime_error("failed to create synchronization objects for a frame!");
    //    }
    //}

}

void Graphic::GlobalInstance::CreateMemoryManager()
{
    VkDeviceSize const k = 1024;
    memoryManager = new Graphic::MemoryManager(32 * k * k);
}

void Graphic::GlobalInstance::CreateVulkanInstance(VulkanInstanceCreator* creator)
{
#ifdef _USE_GRAPHIC_DEBUG
	AddInstanceDebugExtension(creator);
#endif
	AddInstanceWindowparameter(creator);

	VkApplicationInfo applicationInfo{};
	applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	applicationInfo.pApplicationName = creator->applicationName;
	applicationInfo.applicationVersion = creator->applicationVersion;
	applicationInfo.pEngineName = creator->engineName;
	applicationInfo.engineVersion = creator->engineVersion;
	applicationInfo.apiVersion = creator->apiVersion;

	VkInstanceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &applicationInfo;

	createInfo.enabledExtensionCount = static_cast<uint32_t>(creator->_desiredExtensions.size());
	createInfo.ppEnabledExtensionNames = creator->_desiredExtensions.data();

#ifdef _USE_GRAPHIC_DEBUG
	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
	debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	debugCreateInfo.messageSeverity = creator->messageSeverity;
	debugCreateInfo.messageType = creator->messageType;
	debugCreateInfo.pfnUserCallback = creator->debugCallback;

	debugCreateInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
	createInfo.enabledLayerCount = static_cast<uint32_t>(creator->_desiredLayers.size());
	createInfo.ppEnabledLayerNames = creator->_desiredLayers.data();
#endif

	VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);
	if (result != VK_SUCCESS) {
		std::string err = "Failed to create instance, errcode ";
		err += std::to_string(result);
		err += ".";
		throw std::runtime_error(err);
	}
#ifdef _USE_GRAPHIC_DEBUG
	CreateDebugMessenger(creator);
#endif

    CreateWindowSurface();

}

void Graphic::GlobalInstance::CreateVulkanDevice(VulkanDeviceCreator* creator)
{
    AddDeviceWindowParameter(creator);
    
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
    if (deviceCount == 0) throw std::runtime_error("Failed to find GPUs with Vulkan support!");
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    for (const auto& device : devices)
    {
        VkPhysicalDeviceProperties deviceProperties = VkPhysicalDeviceProperties();
        vkGetPhysicalDeviceProperties(device, &deviceProperties);

        if ((deviceProperties.deviceType & creator->desiredPhysicalDeviceType) != creator->desiredPhysicalDeviceType) continue;

#ifdef _USE_GRAPHIC_DEBUG
        uint32_t layerCount = 0;
        uint32_t foundLayerCount = 0;
        vkEnumerateDeviceLayerProperties(device, &layerCount, nullptr);
        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateDeviceLayerProperties(device, &layerCount, availableLayers.data());
        for (const auto& desiredLayer : creator->_desiredDeviceLayers)
        {
            for (const auto& availableLayer : availableLayers)
            {
                if (strcmp(availableLayer.layerName, desiredLayer) == 0)
                {
                    ++foundLayerCount;
                    break;
                }
            }
        }
        if (foundLayerCount != creator->_desiredDeviceLayers.size()) continue;
#endif

        uint32_t extensionCount = 0;
        uint32_t foundExtensionCount = 0;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());
        for (const auto& desiredExtension : creator->_desiredDeviceExtensions)
        {
            for (const auto& availableExtension : availableExtensions)
            {
                if (strcmp(availableExtension.extensionName, desiredExtension) == 0)
                {
                    ++foundExtensionCount;
                    break;
                }
            }
        }
        if (foundExtensionCount != creator->_desiredDeviceExtensions.size()) continue;

        uint32_t queueFamilyCount = 0;
        uint32_t foundQueueCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());
        std::vector<uint32_t> usedCounts = std::vector<uint32_t>(queueFamilyCount, 0);
        std::vector<uint32_t> usedIndexs = std::vector<uint32_t>(creator->_desiredQueues.size(), -1);
        for (uint32_t i = 0; i < creator->_desiredQueues.size(); i++)
        {
            const auto& desiredQueue = creator->_desiredQueues[i];

            if (window && strcmp(desiredQueue.name, "PresentQueue") == 0)
            {
                for (uint32_t j = 0; j < queueFamilies.size(); j++)
                {
                    const auto& queueFamilie = queueFamilies[j];

                    VkBool32 presentSupport = VK_FALSE;
                    vkGetPhysicalDeviceSurfaceSupportKHR(device, j, surface, &presentSupport);
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
        if (foundQueueCount != creator->_desiredQueues.size()) continue;
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
                    queuePrioritieMap[queueIndex].push_back(creator->_desiredQueues[i].prioritie);
                }
                else
                {
                    VkDeviceQueueCreateInfo queueCreateInfo{};
                    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
                    queueCreateInfo.queueFamilyIndex = queueIndex;
                    queueCreateInfo.queueCount = 1;

                    queueCreateInfoMap.insert(std::pair <uint32_t, VkDeviceQueueCreateInfo>(queueIndex, queueCreateInfo));
                    queuePrioritieMap.insert(std::pair < uint32_t, std::vector<float>>(queueIndex, { creator->_desiredQueues[i].prioritie }));
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

        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos = queueCreateInfos.data();

        createInfo.pEnabledFeatures = &creator->_desiredPhysicalDeviceFeatures;

        createInfo.enabledExtensionCount = static_cast<uint32_t>(creator->_desiredDeviceExtensions.size());
        createInfo.ppEnabledExtensionNames = creator->_desiredDeviceExtensions.data();

#ifdef _USE_GRAPHIC_DEBUG
        createInfo.enabledLayerCount = static_cast<uint32_t>(creator->_desiredDeviceLayers.size());
        createInfo.ppEnabledLayerNames = creator->_desiredDeviceLayers.data();
#endif

        physicalDevice = device;

        VkResult result = vkCreateDevice(device, &createInfo, nullptr, &(GlobalInstance::device));
        if (result != VK_SUCCESS) {
            std::string err = "Failed to create logical device, errcode: ";
            err += result;
            err += ".";
            throw std::runtime_error(err);
        }

        usedCounts = std::vector<uint32_t>(queueFamilyCount, 0);
        for (uint32_t i = 0; i < usedIndexs.size(); i++)
        {
            const uint32_t& queueIndex = usedIndexs[i];
            std::string name = std::string(creator->_desiredQueues[i].name);
            Queue* gq = new Queue(name, queueIndex, VK_NULL_HANDLE);
            vkGetDeviceQueue(GlobalInstance::device, queueIndex, usedCounts[queueIndex]++, &(gq->queue));
            GlobalInstance::queues.emplace(name, gq);
        }

        CreateMemoryManager();

        CreateWindowSwapchain();
        CreateWindowSwapchainImages();
        return;
    }
    throw std::runtime_error("Failed to find suitable device.");

}


void Graphic::GlobalInstance::CreateCommandPool(VkCommandPoolCreateFlags flag, const char* queueName, VkCommandPool& commandPool)
{
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = flag;
    poolInfo.queueFamilyIndex = queues[queueName]->queueFamilyIndex;

    if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) 
    {
        throw std::runtime_error("failed to create graphics command pool!");
    }
}

void Graphic::GlobalInstance::CreateCommandBuffer(VkCommandPool& commandPool, VkCommandBufferLevel level, VkCommandBuffer& commandBuffer)
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = level;
    allocInfo.commandBufferCount = 1;

    if (vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer) != VK_SUCCESS) 
    {
        throw std::runtime_error("failed to allocate command buffers!");
    }

}
