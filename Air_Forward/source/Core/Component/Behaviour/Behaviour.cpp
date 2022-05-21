#include "Core/Component/Behaviour/Behaviour.h"
#include <rttr/registration>
RTTR_REGISTRATION
{
	rttr::registration::class_<Core::Component::Behaviour::Behaviour>("Core::Component::Behaviour::Behaviour");
}

Core::Component::Behaviour::Behaviour::Behaviour()
	: Core::Component::Component(Component::ComponentType::BEHAVIOUR)
{
}

Core::Component::Behaviour::Behaviour::~Behaviour()
{
}
