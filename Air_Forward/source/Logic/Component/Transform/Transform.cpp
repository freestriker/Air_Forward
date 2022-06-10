#include "Logic/Component/Transform/Transform.h"
#include "Logic/Object/GameObject.h"
#include <rttr/registration>
#include <glm/glm.hpp>
RTTR_REGISTRATION
{
    using namespace rttr;
    registration::class_<Logic::Component::Transform::Transform>("Logic::Component::Transform::Transform")
        .constructor<>()
        (
            rttr::policy::ctor::as_raw_ptr
        )
        .method("SetTranslation", &Logic::Component::Transform::Transform::SetTranslation)
        .method("SetRotation", &Logic::Component::Transform::Transform::SetRotation)
        .method("SetScale", &Logic::Component::Transform::Transform::SetScale)
        .method("SetTranslationRotationScale", &Logic::Component::Transform::Transform::SetTranslationRotationScale)
        .method("TranslationMatrix", &Logic::Component::Transform::Transform::TranslationMatrix)
        .method("RotationMatrix", &Logic::Component::Transform::Transform::RotationMatrix)
        .method("ScaleMatrix", &Logic::Component::Transform::Transform::ScaleMatrix)
        .method("Translation", &Logic::Component::Transform::Transform::Translation)
        .method("Rotation", &Logic::Component::Transform::Transform::Rotation)
        .method("Scale", &Logic::Component::Transform::Transform::Scale)
        ;
}

Logic::Component::Transform::Transform::Transform()
    : Component(Component::ComponentType::TRANSFORM)
    , _translation(glm::vec3(0, 0, 0))
    , _rotation(glm::vec3(0, 0, 0))
    , _scale(glm::vec3(1, 1, 1))
    , _relativeModelMatrix(glm::mat4(1))
    , _modelMatrix(glm::mat4(1))
{
    
}

Logic::Component::Transform::Transform::~Transform()
{
}

bool Logic::Component::Transform::Transform::Active()
{
    return true;
}

void Logic::Component::Transform::Transform::SetActive()
{
}

void Logic::Component::Transform::Transform::UpdateModelMatrix(glm::mat4& parentModelMatrix)
{
    _modelMatrix = parentModelMatrix * _relativeModelMatrix;
    auto child = _gameObject->Child();
    while (child)
    {
        child->transform.UpdateModelMatrix(_modelMatrix);

        child = child->Brother();
    }
}

void Logic::Component::Transform::Transform::SetTranslation(glm::vec3 translation)
{
    this->_translation = translation;

    _relativeModelMatrix = TranslationMatrix() * RotationMatrix() * ScaleMatrix();
    UpdateModelMatrix(this->_gameObject->Parent()->transform._modelMatrix);
}

void Logic::Component::Transform::Transform::SetRotation(glm::vec3 rotation)
{
    this->_rotation = rotation;

    _relativeModelMatrix = TranslationMatrix() * RotationMatrix() * ScaleMatrix();
    UpdateModelMatrix(this->_gameObject->Parent()->transform._modelMatrix);
}

void Logic::Component::Transform::Transform::SetEulerRotation(glm::vec3 rotation)
{
    double k = std::acos(-1.0) / 180.0;
    this->_rotation = rotation * static_cast<float>(k);

    _relativeModelMatrix = TranslationMatrix() * RotationMatrix() * ScaleMatrix();
    UpdateModelMatrix(this->_gameObject->Parent()->transform._modelMatrix);
}

void Logic::Component::Transform::Transform::SetScale(glm::vec3 scale)
{
    this->_scale = scale;

    _relativeModelMatrix = TranslationMatrix() * RotationMatrix() * ScaleMatrix();
    UpdateModelMatrix(this->_gameObject->Parent()->transform._modelMatrix);
}

void Logic::Component::Transform::Transform::SetTranslationRotationScale(glm::vec3 translation, glm::vec3 rotation, glm::vec3 scale)
{
    this->_translation = translation;
    this->_rotation = rotation;
    this->_scale = scale;

    _relativeModelMatrix = TranslationMatrix() * RotationMatrix() * ScaleMatrix();
    UpdateModelMatrix(this->_gameObject->Parent()->transform._modelMatrix);
}

glm::mat4 Logic::Component::Transform::Transform::TranslationMatrix()
{
    return glm::mat4(
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        _translation.x, _translation.y, _translation.z, 1
    );
}

glm::mat4 Logic::Component::Transform::Transform::RotationMatrix()
{
    return glm::rotate(glm::rotate(glm::rotate(glm::mat4(1), _rotation.x, {1, 0, 0}), _rotation.y, {0, 1, 0}), _rotation.z, {0, 0, 1});
}

glm::mat4 Logic::Component::Transform::Transform::ScaleMatrix()
{
    return glm::mat4(
        _scale.x, 0, 0, 0,
        0, _scale.y, 0, 0,
        0, 0, _scale.z, 0,
        0, 0, 0, 1
    );
}

glm::mat4 Logic::Component::Transform::Transform::ModelMatrix()
{
    return _modelMatrix;
}

glm::vec3 Logic::Component::Transform::Transform::Rotation()
{
    return _rotation;
}

glm::vec3 Logic::Component::Transform::Transform::EulerRotation()
{
    double k = 180.0 / std::acos(-1.0);
    return _rotation * static_cast<float>(k);
}

glm::vec3 Logic::Component::Transform::Transform::Translation()
{
    return _translation;
}

glm::vec3 Logic::Component::Transform::Transform::Scale()
{
    return _scale;
}
