#pragma once
#include <vulkan/vulkan_core.h>
#include <vector>
#include <functional>
#include <mutex>
#include <map>

namespace Graphic
{
	namespace Manager
	{
		class DescriptorSetManager;
		class FrameBufferManager;
		class MemoryManager;
		class RenderPassManager;
	}
	namespace Core
	{
		class Window;
		class Device
		{
			friend class Window;
		public:
			class Queue
			{
			public:
				std::string name;
				uint32_t queueFamilyIndex;
				VkQueue queue;
				std::mutex submitMutex;
				Queue(std::string& name, uint32_t queueFamilyIndex, VkQueue queue) :
					name(name), queueFamilyIndex(queueFamilyIndex), queue(queue), submitMutex()
				{

				}
				~Queue()
				{

				}
			};
			class DeviceCreator
			{
				friend class Graphic::Core::Device;
			private:
				class DesiredQueue final
				{
				public:
					std::string name;
					VkQueueFlags flag;
					float prioritie;
					DesiredQueue(std::string name, VkQueueFlags flag, float prioritie)
						: name(name)
						, flag(flag)
						, prioritie(prioritie)
					{

					}
					~DesiredQueue()
					{

					}
				};
				VkPhysicalDeviceFeatures _desiredPhysicalDeviceFeatures;
				std::vector<std::string> _desiredDeviceExtensions;
				std::vector<DesiredQueue> _desiredQueues;
#ifdef _USE_GRAPHIC_DEBUG
				std::vector<std::string> _desiredDeviceLayers;
#endif
			public:
				VkPhysicalDeviceType desiredPhysicalDeviceType;

			public:
				DeviceCreator();
				~DeviceCreator();
				void AddExtension(std::string extensionName);
				void SetFeature(std::function<void(VkPhysicalDeviceFeatures&)> const& func);
#ifdef _USE_GRAPHIC_DEBUG
				void AddLayer(std::string layerName);
#endif
				void AddQueue(std::string name, VkQueueFlags flag, float prioritie);
			};

			static void Create(Graphic::Core::Device::DeviceCreator& creator);

			static inline VkPhysicalDevice VkPhysicalDevice_();
			static inline VkDevice VkDevice_();
			static inline Graphic::Core::Device::Queue& Queue_(std::string name);
			static inline Graphic::Manager::MemoryManager& MemoryManager();
			static inline Graphic::Manager::RenderPassManager& RenderPassManager();
			static inline Graphic::Manager::DescriptorSetManager& DescriptorSetManager();
			static inline Graphic::Manager::FrameBufferManager& FrameBufferManager();
		private:
			static VkPhysicalDevice _vkPhysicalDevice;
			static VkDevice _vkDevice;
			static std::map<std::string, Queue*> _queues;

			static Graphic::Manager::MemoryManager* _memoryManager;
			static Graphic::Manager::RenderPassManager* _renderPassManager;
			static Graphic::Manager::DescriptorSetManager* _descriptorSetManager;
			static Graphic::Manager::FrameBufferManager* _frameBufferManager;

			static void _AddWindowExtension(Graphic::Core::Device::DeviceCreator& creator);
			static void _CreateManager(Graphic::Core::Device::DeviceCreator& creator);
		};
	}
}


inline VkPhysicalDevice Graphic::Core::Device::VkPhysicalDevice_()
{
	return _vkPhysicalDevice;
}

inline VkDevice Graphic::Core::Device::VkDevice_()
{
	return _vkDevice;
}

inline Graphic::Core::Device::Queue& Graphic::Core::Device::Queue_(std::string name)
{
	return *_queues[name];
}

inline Graphic::Manager::MemoryManager& Graphic::Core::Device::MemoryManager()
{
	return *_memoryManager;
}

inline Graphic::Manager::RenderPassManager& Graphic::Core::Device::RenderPassManager()
{
	return *_renderPassManager;
}

inline Graphic::Manager::DescriptorSetManager& Graphic::Core::Device::DescriptorSetManager()
{
	return *_descriptorSetManager;
}

inline Graphic::Manager::FrameBufferManager& Graphic::Core::Device::FrameBufferManager()
{
	return *_frameBufferManager;
}
