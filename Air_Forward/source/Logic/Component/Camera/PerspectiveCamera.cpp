#include "Logic/Component/Camera/PerspectiveCamera.h"
#include <rttr/registration>
RTTR_REGISTRATION
{
    using namespace rttr;
    registration::class_<Logic::Component::Camera::PerspectiveCamera>("Logic::Component::Camera::PerspectiveCamera");
}

glm::vec4 Logic::Component::Camera::PerspectiveCamera::GetParameter()
{
    const double pi = std::acos(-1.0);
    double halfFov = fovAngle * pi / 360.0;
    float halfHeight = std::tanf(halfFov) * nearFlat;
    float halfWidth = halfHeight * aspectRatio;
    return glm::vec4(halfFov * 2, halfWidth, halfHeight, 0);
}

Logic::Component::Camera::PerspectiveCamera::PerspectiveCamera()
	: Camera(CameraType::PERSPECTIVE)
	, fovAngle(60)
{
    nearFlat = 0.5f;
}

Logic::Component::Camera::PerspectiveCamera::~PerspectiveCamera()
{
}

glm::mat4 Logic::Component::Camera::PerspectiveCamera::ProjectionMatrix()
{
    const double pi = std::acos(-1.0);
    double halfFov = fovAngle * pi / 360.0;
    double cot = 1.0 / std::tan(halfFov);
    float flatDistence = farFlat - nearFlat;

    return glm::mat4(
        cot / aspectRatio   , 0     , 0                                 , 0,
        0                   , cot   , 0                                 , 0,
        0                   , 0     , -farFlat / flatDistence           , -1,
        0                   , 0     , -nearFlat * farFlat / flatDistence, 0
    );
}

std::array<glm::vec4, 6> Logic::Component::Camera::PerspectiveCamera::ClipPlanes()
{
    double pi = std::acos(-1.0);
    double halfFov = fovAngle * pi / 360.0;
    float tanHalfFov = std::tan(halfFov);
    std::array<glm::vec4, 6> planes = std::array<glm::vec4, 6>();
    planes[0] = glm::vec4(0, - nearFlat, -nearFlat * tanHalfFov, 0);
    planes[1] = glm::vec4(0, nearFlat, -nearFlat * tanHalfFov, 0);
    planes[2] = glm::vec4(-nearFlat, 0, -aspectRatio * nearFlat * tanHalfFov, 0);
    planes[3] = glm::vec4(nearFlat, 0, -aspectRatio * nearFlat * tanHalfFov, 0);
    planes[4] = glm::vec4(0, 0, -1, -nearFlat);
    planes[5] = glm::vec4(0, 0, 1, farFlat);
    return planes;
}
