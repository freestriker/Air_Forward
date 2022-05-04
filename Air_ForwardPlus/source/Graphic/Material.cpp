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
		newSlot.slotType = pair.second.slotType;

		_slots.emplace(newSlot.name, newSlot);
	}
}

const Graphic::Texture2D* Graphic::Material::GetTexture2D(const char* name)
{
	if (_slots.count(name) && (_slots[name].slotType == Asset::SlotType::TEXTURE2D || _slots[name].slotType == Asset::SlotType::TEXTURE2D_WITH_INFO))
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
	if (_slots.count(name) && _slots[name].slotType == Asset::SlotType::TEXTURE2D)
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
