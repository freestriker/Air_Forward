#include "Graphic/Material.h"
#include "Graphic/Asset/Shader.h"

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

Graphic::Material::~Material()
{
	delete _shader;
	for (auto& pair : _slots)
	{
		delete pair.second.asset;
	}
	_slots.clear();
}
