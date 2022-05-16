#include "Graphic/Material.h"
#include "Graphic/Asset/Shader.h"
#include "Graphic/Asset/Texture2D.h"
#include "Graphic/Manager/DescriptorSetManager.h"
#include "Graphic/Core/Device.h"
#include "Graphic/Instance/Buffer.h"
#include "Graphic/Instance/DescriptorSet.h"
#include "Graphic/Instance/Image.h"
#include "Graphic/Instance/ImageSampler.h"
#include "Utils/Log.h"

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
		newSlot.descriptorSet = Core::Device::DescriptorSetManager().AcquireDescripterSet(pair.second.slotType, pair.second.descriptorSetLayout);
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
		Utils::Log::Exception("Failed to get texture2d.");
	}
}

void Graphic::Material::SetTexture2D(const char* name, Asset::Texture2D* texture2d)
{
	if (_slots.count(name) && _slots[name].slotType == Asset::SlotType::TEXTURE2D)
	{
		_slots[name].asset = texture2d;
		_slots[name].descriptorSet->UpdateBindingData(
			{ 0 },
			{ {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, texture2d->ImageSampler().VkSampler_(), texture2d->Image().VkImageView_(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL}}
		);
	}
	else if (_slots.count(name) && _slots[name].slotType == Asset::SlotType::TEXTURE2D_WITH_INFO)
	{
		_slots[name].asset = texture2d;
		_slots[name].descriptorSet->UpdateBindingData(
			{ 0, 1 },
			{ 
				{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, texture2d->ImageSampler().VkSampler_(), texture2d->Image().VkImageView_(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL},
				{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, texture2d->TextureInfoBuffer().VkBuffer_(), 0, texture2d->TextureInfoBuffer().Size()}
			}
		);
	}
	else
	{
		Utils::Log::Exception("Failed to set texture2d.");
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
		Utils::Log::Exception("Failed to get uniform buffer.");
	}
}

void Graphic::Material::SetUniformBuffer(const char* name, Graphic::Instance::Buffer* buffer)
{
	if (_slots.count(name) && _slots[name].slotType == Asset::SlotType::UNIFORM_BUFFER)
	{
		_slots[name].asset = buffer;
		_slots[name].descriptorSet->UpdateBindingData(
			{ 0},
			{
				{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, buffer->VkBuffer_(), 0, buffer->Size()}
			}
			);
	}
	else
	{
		Utils::Log::Exception("Failed to set uniform buffer.");
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
			slot.descriptorSet->UpdateBindingData({ 0 }, { Graphic::Instance::DescriptorSet::DescriptorSetWriteData(VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, ub->VkBuffer_(), 0, ub->Size()) });
			break;
		}
		case Asset::SlotType::TEXTURE2D:
		{
			Graphic::Asset::Texture2D* t = static_cast<Asset::Texture2D*>(slot.asset);
			slot.descriptorSet->UpdateBindingData({ 0 }, { Graphic::Instance::DescriptorSet::DescriptorSetWriteData(VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, t->ImageSampler().VkSampler_(), t->Image().VkImageView_(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) });
			break;
		}
		case Asset::SlotType::TEXTURE2D_WITH_INFO:
		{
			Graphic::Asset::Texture2D* t = static_cast<Asset::Texture2D*>(slot.asset);
			slot.descriptorSet->UpdateBindingData({ 0, 1 }, {
				Graphic::Instance::DescriptorSet::DescriptorSetWriteData(VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, t->ImageSampler().VkSampler_(), t->Image().VkImageView_(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL),
				Graphic::Instance::DescriptorSet::DescriptorSetWriteData(VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, t->TextureInfoBuffer().VkBuffer_(), 0, t->TextureInfoBuffer().Size())
				});

			break;
		}
		}

	}
}

VkPipelineLayout Graphic::Material::PipelineLayout()
{
	return this->_shader->VkPipelineLayout();
}

std::vector<VkDescriptorSet> Graphic::Material::DescriptorSets()
{
	std::vector<VkDescriptorSet> sets = std::vector<VkDescriptorSet>(_slots.size());

	for (const auto& slotPair : _slots)
	{
		sets[slotPair.second.set] = slotPair.second.descriptorSet->VkDescriptorSet_();
	}
	return sets;
}

Graphic::Material::~Material()
{
	Asset::Shader::Unload(_shader);
	for (auto& pair : _slots)
	{
		Core::Device::DescriptorSetManager().ReleaseDescripterSet(pair.second.descriptorSet);
	}
	_slots.clear();
}
