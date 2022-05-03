#include "Graphic/Material.h"
#include "Graphic/Asset/Shader.h"
#include "Graphic/Asset/Texture2D.h"

Graphic::Material::Material(Asset::Shader* shader)
	: _shader(shader)
	, _slots()
{
	for (const auto& pair : shader->SlotLayouts())
	{
		_Slot newSlot = _Slot();
		newSlot.asset = nullptr;
		newSlot.name = pair.second.slotName;
		switch (pair.second.slotType)
		{
			case Asset::Shader::SlotLayoutType::TEXTURE2D:
			{
				newSlot.slotType = Material::_SlotType::TEXTURE2D;
				break;
			}
			case Asset::Shader::SlotLayoutType::UNIFORM_BUFFER:
			{
				newSlot.slotType = Material::_SlotType::UNIFORM_BUFFER;
				break;
			}
		}

		_slots.emplace(newSlot.name, newSlot);
	}
}

const Graphic::Texture2D* Graphic::Material::GetTexture2D(const char* name)
{
	if (_slots.count(name) && _slots[name].slotType == Material::_SlotType::TEXTURE2D)
	{
		return static_cast<const Graphic::Texture2D*>(_slots[name].asset);
	}
	else
	{
		throw std::runtime_error("Failed to get texture2d.");
	}
}

void Graphic::Material::SetTexture2D(const char* name, Texture2D* texture2d)
{
	if (_slots.count(name) && _slots[name].slotType == Material::_SlotType::TEXTURE2D)
	{
		_slots[name].asset = texture2d;
	}
	else
	{
		throw std::runtime_error("Failed to get texture2d.");
	}
}

Graphic::Material::~Material()
{
	delete _shader;
	for (auto& pair : _slots)
	{
		delete pair.second.asset;
	}
	_slots.clear();
}
