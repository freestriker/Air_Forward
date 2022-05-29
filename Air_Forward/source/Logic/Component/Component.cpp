#include "Logic/Component/Component.h"
#include "Logic/Object/GameObject.h"
#include "Logic/Component/Behaviour/Behaviour.h"
#include "Logic/Component/Camera/Camera.h"
#include "Logic/Component/Renderer/Renderer.h"
#include <rttr/registration>

RTTR_REGISTRATION
{
	rttr::registration::class_<Logic::Component::Component>("Logic::Component::Component")
		.method("GameObject", &Logic::Component::Component::GameObject);
}

const std::map<rttr::type, Logic::Component::Component::ComponentType> Logic::Component::Component::TYPE_MAP =
std::map<rttr::type, Logic::Component::Component::ComponentType>
({
	{rttr::type::get<Logic::Component::Transform::Transform>(), Logic::Component::Component::ComponentType::TRANSFORM},
	{rttr::type::get<Logic::Component::Behaviour::Behaviour>(), Logic::Component::Component::ComponentType::BEHAVIOUR},
	{rttr::type::get<Logic::Component::Camera::Camera>(), Logic::Component::Component::ComponentType::CAMERA},
	{rttr::type::get<Logic::Component::Renderer::Renderer>(), Logic::Component::Component::ComponentType::RENDERER},
});

const rttr::type Logic::Component::Component::COMPONENT_TYPE = rttr::type::get< Logic::Component::Component>();

Logic::Component::Component::Component()
	: Component(ComponentType::DEFAULT)
{
}

Logic::Component::Component::Component(ComponentType type)
	: _type(type)
	, LifeCycle()
	, Object()
	, ActivableBase()
	, Utils::CrossLinkableNode()
	, _gameObject(nullptr)
{
}

Logic::Component::Component::~Component()
{
}

Logic::Object::GameObject* Logic::Component::Component::GameObject()
{
	return _gameObject;
}

Logic::Component::Component::ComponentType Logic::Component::Component::GetComponentType()
{
	return _type;
}
