#include "Logic/Component/Light/Light.h"
#include <rttr/registration>

RTTR_REGISTRATION
{
	rttr::registration::class_<Logic::Component::Light::Light>("Logic::Component::Light::Light");
}

Logic::Component::Light::Light::Light(LightType lightType)
	: Component(Component::ComponentType::LIGHT)
	, lightType(lightType)
	, color({1, 1, 1, 1})
	, _worldPosition({0, 0, 0})
	, intensity(1.0f)
{
}

Logic::Component::Light::Light::~Light()
{
}