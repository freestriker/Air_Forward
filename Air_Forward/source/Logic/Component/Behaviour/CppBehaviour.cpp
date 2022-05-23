#include "Logic/Component/Behaviour/CppBehaviour.h"
#include <rttr/registration>
RTTR_REGISTRATION
{
	rttr::registration::class_<Logic::Component::Behaviour::CppBehaviour>("Logic::Component::Behaviour::CppBehaviour");
}

Logic::Component::Behaviour::CppBehaviour::CppBehaviour()
	: Behaviour::Behaviour()
{
}

Logic::Component::Behaviour::CppBehaviour::~CppBehaviour()
{
}
