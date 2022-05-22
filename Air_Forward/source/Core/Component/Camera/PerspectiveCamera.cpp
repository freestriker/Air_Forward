#include "Core/Component/Camera/PerspectiveCamera.h"

Core::Component::Camera::PerspectiveCamera::PerspectiveCamera()
	: Camera()
	, fovAngle(60)
{
}

Core::Component::Camera::PerspectiveCamera::~PerspectiveCamera()
{
}

glm::mat4 Core::Component::Camera::PerspectiveCamera::ProjectionMatrix()
{
    return glm::perspective(fovAngle, aspectRatio, -nearFlat, -farFlat);
}

std::array<glm::vec4, 6> Core::Component::Camera::PerspectiveCamera::ClipPlanes()
{
    double pi = std::acos(-1.0);
    double halfFov = fovAngle * pi / 360.0;
    std::array<glm::vec4, 6> planes = std::array<glm::vec4, 6>();
    planes[0] = glm::vec4(-nearFlat, 0, -nearFlat * std::tan(halfFov), 0);
    planes[1] = glm::vec4(nearFlat, 0, -nearFlat * std::tan(halfFov), 0);
    planes[2] = glm::vec4(-nearFlat, 0, -aspectRatio * nearFlat * std::tan(halfFov), 0);
    planes[3] = glm::vec4(nearFlat, 0, -aspectRatio * nearFlat * std::tan(halfFov), 0);
    planes[4] = glm::vec4(0, 0, -1, -nearFlat);
    planes[5] = glm::vec4(0, 0, 1, farFlat);
}
