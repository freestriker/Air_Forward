#pragma once
#include <vulkan/vulkan_core.h>
#include <vector>
#include <map>

namespace Graphic
{
	class DescriptorSetLayoutBinding
	{
	public:
		VkDescriptorType descriptorType;
		VkShaderStageFlags stageFlags;
		DescriptorSetLayoutBinding(VkDescriptorType descriptorType, VkShaderStageFlags stageFlags);
		~DescriptorSetLayoutBinding();
	};

	class DescriptorSetLayout
	{
	private:
		static std::vector<VkDescriptorType> _GetDescriptorTypes(std::vector<DescriptorSetLayoutBinding>& bindings);
		static VkDescriptorSetLayout _CreateDescriptorSetLayout(std::vector<DescriptorSetLayoutBinding>& bindings);
	public:
		DescriptorSetLayout(std::vector<DescriptorSetLayoutBinding> bindings);
		~DescriptorSetLayout();
		VkDescriptorSetLayout const vkDescriptorSetLayout;
		std::vector<VkDescriptorType> const descriptorTypes;
	};

	class DescriptorSet
	{
		friend class DescriptorPool;
	public:
		struct WriteData
		{
			VkDescriptorType type;

			VkBuffer buffer;
			VkDeviceSize offset;
			VkDeviceSize range;

			VkImageLayout layout;
			VkImageView view;
			VkSampler sampler;

			WriteData(VkDescriptorType type, VkBuffer buffer, VkDeviceSize offset, VkDeviceSize range);
			WriteData(VkDescriptorType type, VkImageLayout layout, VkImageView view, VkSampler sampler);
		};
		VkDescriptorPool const sourcePool;
		VkDescriptorSet const descriptorSet;
		void WriteBindingData(std::vector<WriteData> data);
	private:
		DescriptorSet(VkDescriptorPool sourcePool, VkDescriptorSet set);
		~DescriptorSet();
	};

	class DescriptorPool
	{
	private:
		int const _chunkSize;
		DescriptorSetLayout* const _templateLayout;
		std::vector<VkDescriptorPoolSize> const _poolSizes;

		std::vector<VkDescriptorPool> _pools;
		std::map<VkDescriptorPool, int> _poolRemainingCounts;
	public:
		DescriptorPool(DescriptorSetLayout* templateLayout, int chunkSize);
		~DescriptorPool();
		DescriptorSet* GetDescripterSet();
		void RecycleDescripterSet(DescriptorSet* descriptorSet);
	private:
		size_t _CreateNewPool();
		void _DestoryPool(size_t index);
		static std::vector<VkDescriptorPoolSize> _GetPoolSizes(DescriptorSetLayout* templateLayout, int chunkSize);
	};
}