#include "Core/Component/Behaviour/CppBehaviour.h"
#include <rttr/registration>
RTTR_REGISTRATION
{
	rttr::registration::class_<Core::Component::Behaviour::CppBehaviour>("Core::Component::Behaviour::CppBehaviour");
}

Core::Component::Behaviour::CppBehaviour::CppBehaviour()
	: Behaviour::Behaviour()
{
}

Core::Component::Behaviour::CppBehaviour::~CppBehaviour()
{
}
