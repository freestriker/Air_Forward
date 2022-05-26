#include "Logic/Component/Camera/PerspectiveCamera.h"
#include <rttr/registration>
RTTR_REGISTRATION
{
    using namespace rttr;
    registration::class_<Logic::Component::Camera::PerspectiveCamera>("Logic::Component::Camera::PerspectiveCamera");
}

glm::vec4 Logic::Component::Camera::PerspectiveCamera::GetParameter()
{
    return glm::vec4(fovAngle, 0, 0, 0);
}

Logic::Component::Camera::PerspectiveCamera::PerspectiveCamera()
	: Camera(CameraType::PERSPECTIVE)
	, fovAngle(60)
{
}

Logic::Component::Camera::PerspectiveCamera::~PerspectiveCamera()
{
}

glm::mat4 Logic::Component::Camera::PerspectiveCamera::ProjectionMatrix()
{
    return glm::perspective(fovAngle, aspectRatio, -nearFlat, -farFlat);
}

std::array<glm::vec4, 6> Logic::Component::Camera::PerspectiveCamera::ClipPlanes()
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
    return planes;
}
