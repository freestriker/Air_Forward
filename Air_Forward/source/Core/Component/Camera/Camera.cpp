#include "Core/Component/Camera/Camera.h"
#include "Core/Object/GameObject.h"
#include <glm/vec4.hpp>
#include "Utils/Log.h"

void Core::Component::Camera::Camera::OnUpdate()
{
	_modelMatrix = _gameObject->transform.ModelMatrix();
	Utils::Log::Message("Core::Component::Camera::Camera::OnUpdate()");
}

Core::Component::Camera::Camera::Camera()
	: Component(ComponentType::CAMERA)
	, nearFlat(0.1f)
	, farFlat(100.0f)
	, aspectRatio(16.0f / 9.0f)
	, _modelMatrix(glm::mat4(1.0f))
{
}

Core::Component::Camera::Camera::~Camera()
{
}

glm::mat4 Core::Component::Camera::Camera::ViewMatrix()
{
	glm::vec3 eye = _modelMatrix * glm::vec4(0, 0, 0, 1);
	glm::vec3 center = glm::normalize(glm::vec3(_modelMatrix * glm::vec4(0, 0, -1, 1)));
	glm::vec3 up = glm::normalize(glm::vec3(_modelMatrix * glm::vec4(0, 1, 0, 1)));

	return glm::lookAt(eye, center, up);
}