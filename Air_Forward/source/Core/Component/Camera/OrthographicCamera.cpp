#include "Core/Component/Camera/OrthographicCamera.h"
#include <glm/glm.hpp>
#include <glm/ext/matrix_clip_space.hpp>

Core::Component::Camera::OrthographicCamera::OrthographicCamera()
	: Camera()
	, size(225)
{
}

Core::Component::Camera::OrthographicCamera::~OrthographicCamera()
{
}

glm::mat4 Core::Component::Camera::OrthographicCamera::ProjectionMatrix()
{
	float halfWidth = aspectRatio * size;
	float halfHeight = size;

	return glm::ortho(-halfWidth, halfWidth, - halfHeight, halfHeight, -nearFlat, -farFlat);
}

std::array<glm::vec4, 6> Core::Component::Camera::OrthographicCamera::ClipPlanes()
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
