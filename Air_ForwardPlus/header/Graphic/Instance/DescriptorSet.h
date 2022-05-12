#pragma once
#include <vulkan/vulkan_core.h>
#include <vector>
namespace Graphic
{
	namespace Asset
	{
		enum class SlotType;
	}
	namespace Manager
	{
		class DescriptorSetManager;
	}
	namespace Instance
	{
		class DescriptorSet
		{
			friend class Manager::DescriptorSetManager;
		public:
			struct DescriptorSetWriteData
			{
				VkDescriptorType type;
				VkBuffer buffer;
				VkDeviceSize offset;
				VkDeviceSize range;
				VkSampler sampler;
				VkImageView imageView;
				VkImageLayout imageLayout;
				DescriptorSetWriteData(VkDescriptorType type, VkBuffer buffer, VkDeviceSize offset, VkDeviceSize range);
				DescriptorSetWriteData(VkDescriptorType type, VkSampler sampler, VkImageView imageView, VkImageLayout imageLayout);
			};
			VkDescriptorSet VkDescriptorSet_();
			void UpdateBindingData(std::vector<uint32_t> bindingIndex, std::vector<Graphic::Instance::DescriptorSet::DescriptorSetWriteData> data);
		private:
			Asset::SlotType _slotType;
			VkDescriptorSet _vkDescriptorSet;
			VkDescriptorPool _sourceVkDescriptorChunk;
			DescriptorSet(Asset::SlotType slotType, VkDescriptorSet set, VkDescriptorPool sourceDescriptorChunk);
			~DescriptorSet();
			DescriptorSet(const DescriptorSet&) = delete;
			DescriptorSet& operator=(const DescriptorSet&) = delete;
			DescriptorSet(DescriptorSet&&) = delete;
			DescriptorSet& operator=(DescriptorSet&&) = delete;
		};
		typedef DescriptorSet* DescriptorSetHandle;
	}
}