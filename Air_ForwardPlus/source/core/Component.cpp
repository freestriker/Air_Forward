#include "core/Component.h"
#include <rttr/registration>
#include "core/GameObject.h"

RTTR_REGISTRATION
{
	rttr::registration::class_<Component>("Component")
				.constructor<>()
				.property("gameObject", &Component::gameObject);
}

Component::Component(): gameObject(nullptr)
{
}

Component::~Component()
{
	gameObject = nullptr;
}

void Component::UpdateSelf(GameObject* parentGameObject)
{
}

void Component::OnAdd()
{
}
