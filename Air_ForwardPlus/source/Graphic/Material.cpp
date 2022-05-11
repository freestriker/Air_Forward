#include "Graphic/Material.h"
#include "Graphic/Asset/Shader.h"
#include "Graphic/Asset/Texture2D.h"
#include "Graphic/DescriptorSetUtils.h"
#include "Graphic/GlobalInstance.h"
#include "Graphic/Instance/Buffer.h"
#include "Graphic/DescriptorSetUtils.h"

Graphic::Material::Material(Asset::Shader* shader)
	: _shader(shader)
	, _slots()
{
	for (const auto& pair : shader->SlotLayouts())
	{
		_Slot newSlot = _Slot();
		newSlot.asset = nullptr;
		newSlot.name = pair.second.slotName;
		newSlot.slotType = pair.second.slotType;
		newSlot.descriptorSet = Graphic::GlobalInstance::descriptorSetManager->AcquireDescripterSet(pair.second.slotType, pair.second.descriptorSetLayout);
		newSlot.set = pair.second.set;
		_slots.emplace(newSlot.name, newSlot);
	}
}

const Graphic::Asset::Texture2D* Graphic::Material::GetTexture2D(const char* name)
{
	if (_slots.count(name) && (_slots[name].slotType == Asset::SlotType::TEXTURE2D || _slots[name].slotType == Asset::SlotType::TEXTURE2D_WITH_INFO))
	{
		return static_cast<const Graphic::Asset::Texture2D*>(_slots[name].asset);
	}
	else
	{
		throw std::runtime_error("Failed to get texture2d.");
	}
}

void Graphic::Material::SetTexture2D(const char* name, Asset::Texture2D* texture2d)
{
	if (_slots.count(name) && _slots[name].slotType == Asset::SlotType::TEXTURE2D)
	{
		_slots[name].asset = texture2d;
		_slots[name].descriptorSet->WriteBindingData(
			{ 0 },
			{ {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, texture2d->VkSampler(), texture2d->VkImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL} }
		);
	}
	else if (_slots.count(name) && _slots[name].slotType == Asset::SlotType::TEXTURE2D_WITH_INFO)
	{
		_slots[name].asset = texture2d;
		_slots[name].descriptorSet->WriteBindingData(
			{ 0, 1 },
			{ 
				{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, texture2d->VkSampler(), texture2d->VkImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL},
				{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, texture2d->TextureInfoBuffer().VkBuffer(), 0, texture2d->TextureInfoBuffer().Size()}
			}
		);
	}
	else
	{
		throw std::runtime_error("Failed to set texture2d.");
	}
}

const Graphic::Instance::Buffer* Graphic::Material::GetUniformBuffer(const char* name)
{
	if (_slots.count(name) && _slots[name].slotType == Asset::SlotType::UNIFORM_BUFFER)
	{
		return static_cast<const Graphic::Instance::Buffer*>(_slots[name].asset);
	}
	else
	{
		throw std::runtime_error("Failed to get uniform buffer.");
	}
}

void Graphic::Material::SetUniformBuffer(const char* name, Graphic::Instance::Buffer* buffer)
{
	if (_slots.count(name) && _slots[name].slotType == Asset::SlotType::UNIFORM_BUFFER)
	{
		_slots[name].asset = buffer;
		_slots[name].descriptorSet->WriteBindingData(
			{ 0},
			{
				{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, buffer->VkBuffer(), 0, buffer->Size()}
			}
			);
	}
	else
	{
		throw std::runtime_error("Failed to set uniform buffer.");
	}
}

void Graphic::Material::RefreshSlotData(std::vector<std::string> slotNames)
{
	for (const auto& slotName : slotNames)
	{
		const auto& slot = _slots[slotName];
		switch (slot.slotType)
		{
		case Asset::SlotType::UNIFORM_BUFFER:
		{
			Instance::Buffer* ub = static_cast<Instance::Buffer*>(slot.asset);
			slot.descriptorSet->WriteBindingData({ 0 }, { Graphic::Manager::DescriptorSet::DescriptorSetWriteData(VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, ub->VkBuffer(), 0, ub->Size()) });
			break;
		}
		case Asset::SlotType::TEXTURE2D:
		{
			Graphic::Asset::Texture2D* t = static_cast<Asset::Texture2D*>(slot.asset);
			slot.descriptorSet->WriteBindingData({ 0 }, { Graphic::Manager::DescriptorSet::DescriptorSetWriteData(VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, t->VkSampler(), t->VkImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) });
			break;
		}
		case Asset::SlotType::TEXTURE2D_WITH_INFO:
		{
			Graphic::Asset::Texture2D* t = static_cast<Asset::Texture2D*>(slot.asset);
			slot.descriptorSet->WriteBindingData({ 0, 1 }, {
				Graphic::Manager::DescriptorSet::DescriptorSetWriteData(VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, t->VkSampler(), t->VkImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL),
				Graphic::Manager::DescriptorSet::DescriptorSetWriteData(VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, t->TextureInfoBuffer().VkBuffer(), 0, t->TextureInfoBuffer().Size())
				});

			break;
		}
		}

	}
}

VkPipelineLayout Graphic::Material::PipelineLayout()
{
	return this->_shader->PipelineLayout();
}

std::vector<VkDescriptorSet> Graphic::Material::DescriptorSets()
{
	std::vector<VkDescriptorSet> sets = std::vector<VkDescriptorSet>(_slots.size());

	for (const auto& slotPair : _slots)
	{
		sets[slotPair.second.set] = slotPair.second.descriptorSet->Set();
	}
	return sets;
}

Graphic::Material::~Material()
{
	Asset::Shader::Unload(_shader);
	for (auto& pair : _slots)
	{
		Graphic::GlobalInstance::descriptorSetManager->ReleaseDescripterSet(pair.second.descriptorSet);
	}
	_slots.clear();
}
