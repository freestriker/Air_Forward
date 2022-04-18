#pragma once
#include <vulkan/vulkan_core.h>
#include <vector>

namespace Graphic
{
	class DescriptorSetLayout
	{
	public:
		VkDescriptorSetLayout vkDescriptorSetLayout;
		std::vector<VkDescriptorType> descriptorTypes;
		DescriptorSetLayout();
		~DescriptorSetLayout();
	};
	class DescriptorSetLayoutBinding
	{
	public:
		VkDescriptorType descriptorType;
		VkShaderStageFlags stageFlags;
		DescriptorSetLayoutBinding(VkDescriptorType descriptorType, VkShaderStageFlags stageFlags);
		~DescriptorSetLayoutBinding();
	};
	class DescriptorSetLayoutUtils
	{
	public:
		static DescriptorSetLayout CreateDescriptrSetLayout(std::vector<DescriptorSetLayoutBinding> bindings);
		static void DescoryDescriptorSetLayout(DescriptorSetLayout descriptorSetLayout);
	};
}