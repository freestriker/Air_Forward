#pragma once
#include <vulkan/vulkan_core.h>
#include <vector>
#include <map>

namespace Graphic
{
	namespace Asset
	{
		enum class SlotType;
	}
	namespace Manager
	{
		typedef DescriptorSet* DescriptorSetHandle;

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
				std::mutex mutex;
				int const chunkSize;
				std::vector<VkDescriptorPoolSize> const chunkSizes;

				std::vector<VkDescriptorPool> chunks;
				std::map<VkDescriptorPool, int> poolRemainingCounts;
				_DescriptorPool(Asset::SlotType slotType, std::vector< VkDescriptorType>& types, int chunkSize);
				~_DescriptorPool();
				DescriptorSet* AcquireDescripterSet(VkDescriptorSetLayout descriptorSetLayout);
				void ReleaseDescripterSet(DescriptorSet* descriptorSet);
				size_t CreateNewPool();
				void DestoryPool(size_t index);
				static std::vector<VkDescriptorPoolSize> GetPoolSizes(std::vector< VkDescriptorType>& types, int chunkSize);
			};

		};

	}




}