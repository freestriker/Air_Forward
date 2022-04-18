#include "Graphic/DescriptorSetUtils.h"
#include "Graphic/GlobalInstance.h"
#include <stdexcept>

Graphic::DescriptorSetLayout::DescriptorSetLayout()
	: vkDescriptorSetLayout(VK_NULL_HANDLE)
	, descriptorTypes()
{
}

Graphic::DescriptorSetLayout::~DescriptorSetLayout()
{
	vkDescriptorSetLayout = VK_NULL_HANDLE;
}

Graphic::DescriptorSetLayout Graphic::DescriptorSetLayoutUtils::CreateDescriptrSetLayout(std::vector<Graphic::DescriptorSetLayoutBinding> bindings)
{
	auto result = DescriptorSetLayout();
	result.descriptorTypes.resize(bindings.size());

	std::vector<VkDescriptorSetLayoutBinding> vkBindings = std::vector<VkDescriptorSetLayoutBinding>(bindings.size());
	for (size_t i = 0; i < bindings.size(); i++)
	{
		vkBindings[i].binding = static_cast<uint32_t>(i);
		vkBindings[i].descriptorCount = 1;
		vkBindings[i].descriptorType = bindings[i].descriptorType;
		vkBindings[i].pImmutableSamplers = nullptr;
		vkBindings[i].stageFlags = bindings[i].stageFlags;

		result.descriptorTypes[i] = bindings[i].descriptorType;
	}

	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32_t>(vkBindings.size());
	layoutInfo.pBindings = vkBindings.data();

	if (vkCreateDescriptorSetLayout(Graphic::GlobalInstance::device, &layoutInfo, nullptr, &result.vkDescriptorSetLayout) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create descriptor set layout!");
	}

	return result;
}

void Graphic::DescriptorSetLayoutUtils::DescoryDescriptorSetLayout(DescriptorSetLayout descriptorSetLayout)
{
	vkDestroyDescriptorSetLayout(Graphic::GlobalInstance::device, descriptorSetLayout.vkDescriptorSetLayout, nullptr);

}

Graphic::DescriptorSetLayoutBinding::DescriptorSetLayoutBinding(VkDescriptorType descriptorType, VkShaderStageFlags stageFlags)
	: descriptorType(descriptorType)
	, stageFlags(stageFlags)
{
}

Graphic::DescriptorSetLayoutBinding::~DescriptorSetLayoutBinding()
{
}
