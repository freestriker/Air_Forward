#include "Test/PointLightMoveBehaviour.h"
#include "Logic/Component/Renderer/MeshRenderer.h"
#include "Logic/Object/GameObject.h"
#include "Logic/Component/Transform/Transform.h"
#include "Logic/Core/Instance.h"
#include <algorithm>
#include <rttr/registration>
RTTR_REGISTRATION
{
	rttr::registration::class_<Test::PointLightMoveBehaviour>("Test::PointLightMoveBehaviour");
}



Test::PointLightMoveBehaviour::PointLightMoveBehaviour(float eulerAngularVelocity)
	: _eulerAngularVelocity(eulerAngularVelocity)
{
}

Test::PointLightMoveBehaviour::~PointLightMoveBehaviour()
{
}

void Test::PointLightMoveBehaviour::OnAwake()
{
}

void Test::PointLightMoveBehaviour::OnStart()
{

}

void Test::PointLightMoveBehaviour::OnUpdate()
{
	auto rotationAngle = _eulerAngularVelocity * Logic::Core::Instance::time.DeltaDuration();
	auto curRotation = GameObject()->transform.EulerRotation();
	curRotation.x = std::fmod(curRotation.x + rotationAngle * 1.0, 360);
	curRotation.y = std::fmod(curRotation.y + rotationAngle * 0.6, 360);
	curRotation.z = std::fmod(curRotation.z + rotationAngle * 0.3, 360);
	GameObject()->transform.SetEulerRotation(curRotation);
}

void Test::PointLightMoveBehaviour::OnDestroy()
{
}
