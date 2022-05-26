#include "Logic/Component/Camera/OrthographicCamera.h"
#include <glm/glm.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <rttr/registration>
RTTR_REGISTRATION
{
	using namespace rttr;
	registration::class_<Logic::Component::Camera::OrthographicCamera>("Logic::Component::Camera::OrthographicCamera");
}

glm::vec4 Logic::Component::Camera::OrthographicCamera::GetParameter()
{
	return glm::vec4(size, 0, 0, 0);
}

Logic::Component::Camera::OrthographicCamera::OrthographicCamera()
	: Camera(CameraType::ORTHOGRAPHIC)
	, size(2.25f)
{
}

Logic::Component::Camera::OrthographicCamera::~OrthographicCamera()
{
}

glm::mat4 Logic::Component::Camera::OrthographicCamera::ProjectionMatrix()
{
	float halfWidth = aspectRatio * size;
	float halfHeight = size;
	float flatDistence = farFlat - nearFlat;

	return glm::mat4(
		1.0 / halfWidth, 0, 0, 0,
		0, 1.0 / halfHeight, 0, 0,
		0, 0, -1.0 / flatDistence, 0,
		0, 0, -nearFlat / flatDistence, 1
	);
}

std::array<glm::vec4, 6> Logic::Component::Camera::OrthographicCamera::ClipPlanes()
{
	std::array<glm::vec4, 6> planes = std::array<glm::vec4, 6>();
	planes[0] = glm::vec4(-1, 0, 0, aspectRatio * size);
	planes[1] = glm::vec4(1, 0, 0, aspectRatio * size);
	planes[2] = glm::vec4(0, -1, 0, size);
	planes[3] = glm::vec4(0, 1, 0, size);
	planes[4] = glm::vec4(0, 0, -1, -nearFlat);
	planes[5] = glm::vec4(0, 0, 1, farFlat);
	return planes;
}
