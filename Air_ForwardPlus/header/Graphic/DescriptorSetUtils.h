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
			struct DescriptorSetWriteData
			{
				VkDescriptorType type;
				VkBuffer        buffer;
				VkDeviceSize    offset;
				VkDeviceSize    range;
				VkSampler        sampler;
				VkImageView      imageView;
				VkImageLayout    imageLayout;
				DescriptorSetWriteData(VkDescriptorType type, VkBuffer buffer, VkDeviceSize offset, VkDeviceSize range);
				DescriptorSetWriteData(VkDescriptorType type, VkSampler sampler, VkImageView imageView, VkImageLayout imageLayout);
			};
			VkDescriptorSet Set();
			VkDescriptorSetLayout SetLayout();
			void WriteBindingData(std::vector<uint32_t> bindingIndex, std::vector<Graphic::Manager::DescriptorSet::DescriptorSetWriteData> data);
		private:
			Asset::SlotType _slotType;
			VkDescriptorSet _descriptorSet; 
			VkDescriptorSetLayout _descriptorSetLayout;
			VkDescriptorPool _sourceDescriptorChunk;
			DescriptorSet(Asset::SlotType slotType, VkDescriptorSet set, VkDescriptorSetLayout descriptorSetLayout, VkDescriptorPool sourceDescriptorChunk);
			~DescriptorSet();
		};
		typedef DescriptorSet* DescriptorSetHandle;
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
				DescriptorSetHandle AcquireDescripterSet(VkDescriptorSetLayout descriptorSetLayout);
				void ReleaseDescripterSet(DescriptorSetHandle descriptorSet);
				void CollectEmptyChunk();
				
				static std::vector<VkDescriptorPoolSize> GetPoolSizes(std::vector< VkDescriptorType>& types, int chunkSize);
				static VkDescriptorPoolCreateInfo GetChunkCreateInfo(uint32_t chunkSize, std::vector<VkDescriptorPoolSize> const& chunkSizes);
			};
			std::shared_mutex _managerMutex;
			std::map< Asset::SlotType, _DescriptorPool*> _pools;
		public:
			void AddDescriptorSetPool(Asset::SlotType slotType, std::vector< VkDescriptorType> descriptorTypes, uint32_t chunkSize);
			void DeleteDescriptorSetPool(Asset::SlotType slotType);
			DescriptorSetHandle AcquireDescripterSet(Asset::SlotType slotType, VkDescriptorSetLayout descriptorSetLayout);
			void ReleaseDescripterSet(DescriptorSetHandle descriptorSet);
			void Collect();

			DescriptorSetManager();
			~DescriptorSetManager();
		};

	}




}