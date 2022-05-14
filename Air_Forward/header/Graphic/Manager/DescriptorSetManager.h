#pragma once
#include <vulkan/vulkan_core.h>
#include <vector>
#include <map>
#include <mutex>
#include <shared_mutex>
#include <set>

namespace Graphic
{
	namespace Asset
	{
		enum class SlotType;
	}
	namespace Instance
	{
		class DescriptorSet;
		typedef DescriptorSet* DescriptorSetHandle;
	}
	namespace Manager
	{
		class DescriptorSetManager
		{
		private:
			class _DescriptorPool
			{
			public:
				Asset::SlotType const slotType;
				uint32_t const chunkSize;
				std::vector<VkDescriptorPoolSize> const poolSizes;
				VkDescriptorPoolCreateInfo const chunkCreateInfo;

				std::mutex mutex;

				std::map<VkDescriptorPool, uint32_t> chunks;

				std::set<Instance::DescriptorSetHandle> handles;

				_DescriptorPool(Asset::SlotType slotType, std::vector< VkDescriptorType>& types, uint32_t chunkSize);
				~_DescriptorPool();
				Instance::DescriptorSetHandle AcquireDescripterSet(VkDescriptorSetLayout descriptorSetLayout);
				void ReleaseDescripterSet(Instance::DescriptorSetHandle descriptorSet);
				void CollectEmptyChunk();
				
				static std::vector<VkDescriptorPoolSize> GetPoolSizes(std::vector< VkDescriptorType>& types, int chunkSize);
				static VkDescriptorPoolCreateInfo GetChunkCreateInfo(uint32_t chunkSize, std::vector<VkDescriptorPoolSize> const& chunkSizes);
			};
			std::shared_mutex _managerMutex;
			std::map< Asset::SlotType, _DescriptorPool*> _pools;
		public:
			void AddDescriptorSetPool(Asset::SlotType slotType, std::vector< VkDescriptorType> descriptorTypes, uint32_t chunkSize);
			void DeleteDescriptorSetPool(Asset::SlotType slotType);
			Instance::DescriptorSetHandle AcquireDescripterSet(Asset::SlotType slotType, VkDescriptorSetLayout descriptorSetLayout);
			void ReleaseDescripterSet(Instance::DescriptorSetHandle descriptorSet);
			void Collect();

			DescriptorSetManager();
			~DescriptorSetManager();
		};

	}




}