#include "Core/Component/Transform/Transform.h"
#include "Core/Object/GameObject.h"
#include <rttr/registration>
RTTR_REGISTRATION
{
    using namespace rttr;
    registration::class_<Core::Component::Transform::Transform>("Core::Component::Transform::Transform")
        .constructor<>()
        (
            rttr::policy::ctor::as_raw_ptr
        )
        .method("SetTranslation", &Core::Component::Transform::Transform::SetTranslation)
        .method("SetRotation", &Core::Component::Transform::Transform::SetRotation)
        .method("SetScale", &Core::Component::Transform::Transform::SetScale)
        .method("SetTranslationRotationScale", &Core::Component::Transform::Transform::SetTranslationRotationScale)
        .method("TranslationMatrix", &Core::Component::Transform::Transform::TranslationMatrix)
        .method("RotationMatrix", &Core::Component::Transform::Transform::RotationMatrix)
        .method("ScaleMatrix", &Core::Component::Transform::Transform::ScaleMatrix)
        .method("Translation", &Core::Component::Transform::Transform::Translation)
        .method("Rotation", &Core::Component::Transform::Transform::Rotation)
        .method("Scale", &Core::Component::Transform::Transform::Scale)
        ;
}

Core::Component::Transform::Transform::Transform()
    : Component(Component::ComponentType::TRANSFORM)
    , _translation(glm::vec3(0, 0, 0))
    , _rotation(glm::quat(1, 0, 0, 0))
    , _scale(glm::vec3(1, 1, 1))
{

}

Core::Component::Transform::Transform::~Transform()
{
}

bool Core::Component::Transform::Transform::Active()
{
    return true;
}

void Core::Component::Transform::Transform::SetActive()
{
}

void Core::Component::Transform::Transform::SetTranslation(glm::vec3 translation)
{
    this->_translation = translation;
}

void Core::Component::Transform::Transform::SetRotation(glm::quat rotation)
{
    this->_rotation = rotation;
}

void Core::Component::Transform::Transform::SetScale(glm::vec3 scale)
{
    this->_scale = scale;
}

void Core::Component::Transform::Transform::SetTranslationRotationScale(glm::vec3 translation, glm::quat rotation, glm::vec3 scale)
{
    this->_translation = translation;
    this->_rotation = rotation;
    this->_scale = scale;
}

glm::mat4 Core::Component::Transform::Transform::TranslationMatrix()
{
    return glm::mat4(
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        _translation.x, _translation.y, _translation.z, 1
    );
}

glm::mat4 Core::Component::Transform::Transform::RotationMatrix()
{
    return glm::mat4_cast(_rotation);
}

glm::mat4 Core::Component::Transform::Transform::ScaleMatrix()
{
    return glm::mat4(
        _scale.x, 0, 0, 0,
        0, _scale.y, 0, 0,
        0, 0, _scale.z, 0,
        0, 0, 0, 1
    );
}

glm::quat Core::Component::Transform::Transform::Rotation()
{
    return _rotation;
}

glm::vec3 Core::Component::Transform::Transform::Translation()
{
    return _translation;
}

glm::vec3 Core::Component::Transform::Transform::Scale()
{
    return _scale;
}
