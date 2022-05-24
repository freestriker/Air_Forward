#include "Logic/Component/Light/PointLight.h"
#include "Logic/Component/Transform/Transform.h"
#include "Logic/Object/GameObject.h"
#include <rttr/registration>

RTTR_REGISTRATION
{
	rttr::registration::class_<Logic::Component::Light::PointLight>("Logic::Component::Light::PointLight");
}

Logic::Component::Light::Light::LightData Logic::Component::Light::PointLight::GetLightData()
{
	LightData lightDate{};
	lightDate.type.x = 2;
	lightDate.type.y = intensity;
	lightDate.type.z = range;
	lightDate.position = _worldPosition;
	lightDate.color = color;
	return lightDate;
}

Logic::Component::Light::PointLight::PointLight()
	: Light(Light::LightType::POINT)
	, range(10.0f)
{
}

Logic::Component::Light::PointLight::~PointLight()
{
}

void Logic::Component::Light::PointLight::OnUpdate()
{
	_worldPosition = GameObject()->transform.ModelMatrix() * glm::vec4(0, 0, 0, 1);
}
