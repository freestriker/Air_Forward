#include "Component/Transform/Transform.h"
#include "core/GameObject.h"
#include <rttr/registration>
RTTR_REGISTRATION
{
    using namespace rttr;
    registration::class_<Transform>("Transform")
        .constructor<>()
        (
            rttr::policy::ctor::as_raw_ptr
        )
        .method("SetTranslation", &Transform::SetTranslation)
        .method("SetRotation", &Transform::SetRotation)
        .method("SetScale", &Transform::SetScale)
        .method("SetTranslationRotationScale", &Transform::SetTranslationRotationScale)
        .method("TranslationMatrix", &Transform::TranslationMatrix)
        .method("RotationMatrix", &Transform::RotationMatrix)
        .method("ScaleMatrix", &Transform::ScaleMatrix)
        ;
}


void Transform::SetTranslation(glm::vec3 translation)
{
    this->translation = translation;
}

void Transform::SetRotation(glm::quat rotation)
{
    this->rotation = rotation;
}

void Transform::SetScale(glm::vec3 scale)
{
    this->scale = scale;
}

void Transform::SetTranslationRotationScale(glm::vec3 translation, glm::quat rotation, glm::vec3 scale)
{
    this->translation = translation;
    this->rotation = rotation;
    this->scale = scale;
}

glm::mat4 Transform::TranslationMatrix()
{
    return glm::mat4(
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        translation.x, translation.y, translation.z, 1
    );
}

glm::mat4 Transform::RotationMatrix()
{
    return glm::mat4_cast(rotation);
}

glm::mat4 Transform::ScaleMatrix()
{
    return glm::mat4(
        scale.x, 0, 0, 0,
        0, scale.y, 0, 0,
        0, 0, scale.z, 0,
        0, 0, 0, 1
    );
}

Transform::Transform(): translation(glm::vec3(0)), rotation(glm::quat(1, 0, 0, 0)), scale(glm::vec3(1))
{
}

Transform::~Transform()
{
}