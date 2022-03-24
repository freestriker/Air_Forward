#include "core/Component.h"
#include <rttr/registration>

RTTR_REGISTRATION
{
	rttr::registration::class_<Component>("Component")
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
