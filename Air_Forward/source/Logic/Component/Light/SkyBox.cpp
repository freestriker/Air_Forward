#include "Logic/Component/Light/SkyBox.h"
#include <rttr/registration>
#include "Graphic/Asset/TextureCube.h"

RTTR_REGISTRATION
{
	rttr::registration::class_<Logic::Component::Light::SkyBox>("Logic::Component::Light::SkyBox");
}

Logic::Component::Light::Light::LightData Logic::Component::Light::SkyBox::GetLightData()
{
	LightData lightDate{};
	lightDate.type = 3;
	lightDate.intensity = intensity;
	lightDate.range = 0;
	lightDate.extraParamter = 0;
	lightDate.position = glm::vec3(0);
	lightDate.color = color;
	return lightDate;
}

Graphic::Asset::TextureCube* Logic::Component::Light::SkyBox::TextureCube()
{
	return _textureCube;
}

Logic::Component::Light::SkyBox::SkyBox()
	: Light(LightType::SKY_BOX)
	, skyBoxName("..\\Asset\\Texture\\DefaultTextureCube.json")
	, _textureCube(nullptr)
{
}

Logic::Component::Light::SkyBox::~SkyBox()
{
}

void Logic::Component::Light::SkyBox::OnStart()
{
	_textureCube = Graphic::Asset::TextureCube::Load("..\\Asset\\Texture\\DefaultTextureCube.json");
}

void Logic::Component::Light::SkyBox::OnUpdate()
{

}
