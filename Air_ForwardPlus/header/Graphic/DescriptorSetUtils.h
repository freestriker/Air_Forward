#pragma once
#include <vulkan/vulkan_core.h>
#include <vector>
#include <map>
#include <mutex>
#include <shared_mutex>

namespace Graphic
{
	namespace Asset
	{
		enum class SlotType;
	}
	namespace Manager
	{
		class DescriptorSet
		{
			friend class DescriptorSetManager;
		public:
			VkDescriptorSet Set();
			VkDescriptorSetLayout SetLayout();
		private:
			Asset::SlotType _slotType;
			VkDescriptorSet _descriptorSet; 
			VkDescriptorSetLayout _descriptorSetLayout;
			VkDescriptorPool _sourceDescriptorChunk;
			DescriptorSet(Asset::SlotType slotType, VkDescriptorSet set, VkDescriptorSetLayout descriptorSetLayout, VkDescriptorPool sourceDescriptorChunk);
			~DescriptorSet();
		};
		class DescriptorSetManager
		{
		private:
			class _DescriptorPool
			{
			public:
				Asset::SlotType const slotType;
				uint32_t const chunkSize;
				std::vector<VkDescriptorPoolSize> const chunkSizes;
				VkDescriptorPoolCreateInfo const chunkCreateInfo;

				std::mutex mutex;

				std::map<VkDescriptorPool, uint32_t> chunks;


				_DescriptorPool(Asset::SlotType slotType, std::vector< VkDescriptorType>& types, uint32_t chunkSize);
				~_DescriptorPool();
				DescriptorSet* AcquireDescripterSet(VkDescriptorSetLayout descriptorSetLayout);
				void ReleaseDescripterSet(DescriptorSet* descriptorSet);
				void CollectEmptyChunk();
				
				static std::vector<VkDescriptorPoolSize> GetPoolSizes(std::vector< VkDescriptorType>& types, int chunkSize);
				static VkDescriptorPoolCreateInfo GetChunkCreateInfo(uint32_t chunkSize, std::vector<VkDescriptorPoolSize> const& chunkSizes);
			};
			std::shared_mutex _managerMutex;
			std::map< Asset::SlotType, _DescriptorPool*> _pools;
		public:
			void AddDescriptorSetPool(Asset::SlotType slotType, std::vector< VkDescriptorType> descriptorTypes, uint32_t chunkSize);
			void DeleteDescriptorSetPool(Asset::SlotType slotType);
			DescriptorSet* AcquireDescripterSet(Asset::SlotType slotType, VkDescriptorSetLayout descriptorSetLayout);
			void ReleaseDescripterSet(DescriptorSet* descriptorSet);
			void Collect();

			DescriptorSetManager();
			~DescriptorSetManager();
		};

	}




}