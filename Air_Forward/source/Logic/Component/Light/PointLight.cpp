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
	lightDate.type = 2;
	lightDate.intensity = intensity;
	lightDate.range = minRange;
	lightDate.extraParamter = maxRange;
	lightDate.position = _worldPosition;
	lightDate.color = color;
	return lightDate;
}

Logic::Component::Light::PointLight::PointLight()
	: Light(Light::LightType::POINT)
	, minRange(1.0f)
	, maxRange(10.0f)
{
}

Logic::Component::Light::PointLight::~PointLight()
{
}

void Logic::Component::Light::PointLight::OnUpdate()
{
	_worldPosition = GameObject()->transform.ModelMatrix() * glm::vec4(0, 0, 0, 1);
}
