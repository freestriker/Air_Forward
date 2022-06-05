#include "Test/CameraMoveBehaviour.h"
#include "Logic/Component/Renderer/MeshRenderer.h"
#include "Logic/Object/GameObject.h"
#include "Logic/Core/Instance.h"
#include <algorithm>
#include <Graphic/Asset/TextureCube.h>
#include <rttr/registration>
RTTR_REGISTRATION
{
	rttr::registration::class_<Test::CameraMoveBehaviour>("Test::CameraMoveBehaviour");
}

Test::CameraMoveBehaviour::CameraMoveBehaviour()
	: _rotation(0)
{
}

Test::CameraMoveBehaviour::~CameraMoveBehaviour()
{
}

void Test::CameraMoveBehaviour::OnAwake()
{
}

void Test::CameraMoveBehaviour::OnStart()
{

}

void Test::CameraMoveBehaviour::OnUpdate()
{
	const double pi = std::acos(-1.0);
	_rotation = std::fmod((_rotation + 15 * Logic::Core::Instance::time.DeltaDuration()), 360.0f);
	float x = std::cos(_rotation / 180 * pi) * 7;
	float y = -std::sin(_rotation / 180 * pi) * 7;
	GameObject()->transform.SetTranslation({ x, y, 0 });
	GameObject()->transform.SetEulerRotation({ 90, 90 - _rotation, -(0) });
}

void Test::CameraMoveBehaviour::OnDestroy()
{
}
