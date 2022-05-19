#include "Core/Component/Component.h"
#include <rttr/registration>
#include "Core/Object/GameObject.h"
#include "Core/Component/Behaviour/Behaviour.h"

RTTR_REGISTRATION
{
	rttr::registration::class_<Core::Component::Component>("Core::Component::Component")
		.method("GameObject", &Core::Component::Component::GameObject);
}

const std::map<rttr::type, Core::Component::Component::ComponentType> Core::Component::Component::TYPE_MAP =
std::map<rttr::type, Core::Component::Component::ComponentType>
({
	{rttr::type::get<Core::Component::Transform::Transform>(), Core::Component::Component::ComponentType::TRANSFORM},
	{rttr::type::get<Core::Component::Behaviour::Behaviour>(), Core::Component::Component::ComponentType::BEHAVIOUR},
});

const rttr::type Core::Component::Component::COMPONENT_TYPE = rttr::type::get< Core::Component::Component>();

Core::Component::Component::Component()
	: Component(ComponentType::DEFAULT)
{
}

Core::Component::Component::Component(ComponentType type)
	: _type(type)
	, LifeCycle()
	, Object()
	, ActivableBase()
	, Utils::CrossLinkableNode()
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

Core::Component::Component::ComponentType Core::Component::Component::GetComponentType()
{
	return _type;
}
