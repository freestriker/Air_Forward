#include "Logic/Component/Light/DirectionalLight.h"
#include "Logic/Component/Transform/Transform.h"
#include "Logic/Object/GameObject.h"
#include <rttr/registration>

RTTR_REGISTRATION
{
	rttr::registration::class_<Logic::Component::Light::DirectionalLight>("Logic::Component::Light::DirectionalLight");
}

Logic::Component::Light::Light::LightData Logic::Component::Light::DirectionalLight::GetLightData()
{
	LightData lightDate{};
	lightDate.type = 1;
	lightDate.intensity = intensity;
	lightDate.range = 0;
	lightDate.extraParamter = 0;
	lightDate.position = _worldPosition;
	lightDate.color = color;
	return lightDate;
}

Logic::Component::Light::DirectionalLight::DirectionalLight()
	: Light(Light::LightType::DIRECTIONAL)
{
}

Logic::Component::Light::DirectionalLight::~DirectionalLight()
{
}

void Logic::Component::Light::DirectionalLight::OnUpdate()
{
	_worldPosition = glm::normalize(glm::vec3(GameObject()->transform.ModelMatrix() * glm::vec4(0, 0, -1, 0)));
}
