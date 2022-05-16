#include "Core/Component/Component.h"
#include <rttr/registration>
#include "Core/Object/GameObject.h"

RTTR_REGISTRATION
{
	rttr::registration::class_<Core::Component::Component>("Core::Component::Component")
		.method("GameObject", &Core::Component::Component::GameObject);
}

Core::Component::Component::Component()
	: LifeCycle()
	, Object()
	, ActivableBase()
	, _gameObject(nullptr)
{
}

Core::Component::Component::~Component()
{
}

Core::Object::GameObject* Core::Component::Component::GameObject()
{
	return _gameObject;
}
