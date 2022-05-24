#include "Logic/Component/Behaviour/Behaviour.h"
#include <rttr/registration>
RTTR_REGISTRATION
{
	rttr::registration::class_<Logic::Component::Behaviour::Behaviour>("Logic::Component::Behaviour::Behaviour");
}

Logic::Component::Behaviour::Behaviour::Behaviour()
	: Logic::Component::Component(Component::ComponentType::BEHAVIOUR)
{
}

Logic::Component::Behaviour::Behaviour::~Behaviour()
{
}
