#include "Logic/Component/Camera/Camera.h"
#include "Logic/Object/GameObject.h"
#include <glm/vec4.hpp>
#include "Utils/Log.h"
#include <rttr/registration>
RTTR_REGISTRATION
{
	using namespace rttr;
	registration::class_<Logic::Component::Camera::Camera>("Logic::Component::Camera::Camera");
}

void Logic::Component::Camera::Camera::OnUpdate()
{
	_modelMatrix = _gameObject->transform.ModelMatrix();
	Utils::Log::Message("Logic::Component::Camera::Camera::OnUpdate()");
}

Logic::Component::Camera::Camera::Camera()
	: Component(ComponentType::CAMERA)
	, nearFlat(0.1f)
	, farFlat(100.0f)
	, aspectRatio(16.0f / 9.0f)
	, _modelMatrix(glm::mat4(1.0f))
{
}

Logic::Component::Camera::Camera::~Camera()
{
}

glm::mat4 Logic::Component::Camera::Camera::ViewMatrix()
{
	glm::vec3 eye = _modelMatrix * glm::vec4(0, 0, 0, 1);
	glm::vec3 center = glm::normalize(glm::vec3(_modelMatrix * glm::vec4(0, 0, -1, 1)));
	glm::vec3 up = glm::normalize(glm::vec3(_modelMatrix * glm::vec4(0, 1, 0, 1)));

	return glm::lookAt(eye, center, up);
}

const glm::mat4& Logic::Component::Camera::Camera::ModelMatrix()
{
	return _modelMatrix;
}
