#include "core/Component.h"
#include <rttr/registration>
#include "core/GameObject.h"
RTTR_REGISTRATION
{
	rttr::registration::class_<Component>("Component")
				.constructor<>()
				.method("Active", &Component::Active)
				.method("SetActive", &Component::SetActive)
				.method("GameObject", &Component::GameObject);
}

Component::Component(): LifeTime(), gameObject(nullptr), active(true)
{
}

Component::~Component()
{
	gameObject = nullptr;
	active = false;
}

bool Component::Active()
{
	return active;
}


void Component::SetActive(bool active)
{
	if (gameObject && gameObject->Active())
	{
		if (this->active != active)
		{
			this->active = active;
			if (active)
			{
				OnEnable();
			}
			else
			{
				OnDisable();
			}
		}
	}
}

GameObject* Component::GameObject()
{
	return gameObject;
}
