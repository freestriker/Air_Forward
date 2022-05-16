#include "Utils/ActivableBase.h"
#include <rttr/registration>

RTTR_REGISTRATION
{
	rttr::registration::class_<Utils::ActivableBase>("Utils::ActivableBase")
		.method("Active", &Utils::ActivableBase::Active)
		.method("SetActive", &Utils::ActivableBase::SetActive);
}

Utils::ActivableBase::ActivableBase()
	: ActivableBase(true)
{
}

Utils::ActivableBase::ActivableBase(bool active)
	: _active(active)
{
}

Utils::ActivableBase::~ActivableBase()
{
}

bool Utils::ActivableBase::Active()
{
	return _active;
}

void Utils::ActivableBase::SetActive(bool active)
{
	_active = active;
}
