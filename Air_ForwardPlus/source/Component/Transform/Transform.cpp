#include "Component/Transform/Transform.h"
#include "core/GameObject.h"

void Transform::SetTranslation(glm::vec3 translation)
{
    this->translation = translation;

    GameObject* parentGameObject = this->gameObject ? this->gameObject->parent : nullptr;
    UpdateSelf(parentGameObject);
    UpdateGameObject(parentGameObject);

}

void Transform::SetRotation(glm::quat rotation)
{
    this->rotation = rotation;

    GameObject* parentGameObject = this->gameObject ? this->gameObject->parent : nullptr;
    UpdateSelf(parentGameObject);
    UpdateGameObject(parentGameObject);

}

void Transform::SetScale(glm::vec3 scale)
{
    this->scale = scale;

    GameObject* parentGameObject = this->gameObject ? this->gameObject->parent : nullptr;
    UpdateSelf(parentGameObject);
    UpdateGameObject(parentGameObject);

}

void Transform::SetTranslationRotationScale(glm::vec3 translation, glm::quat rotation, glm::vec3 scale)
{
    this->translation = translation;
    this->rotation = rotation;
    this->scale = scale;

    GameObject* parentGameObject = this->gameObject ? this->gameObject->parent : nullptr;
    UpdateSelf(parentGameObject);
    UpdateGameObject(parentGameObject);
}

void Transform::UpdateSelf(GameObject* parentGameObject)
{
    GameObject* parentGameObject = static_cast<GameObject*>(data);

    this->modelMatrix = TranslationMatrix() * RotationMatrix() * ScaleMatrix();
    this->worldMatrix = parentGameObject ? parentGameObject->transform.worldMatrix * this->modelMatrix : this->modelMatrix;
}

void Transform::UpdateGameObject(GameObject* parentGameObject)
{
    this->gameObject->UpdateSelfWithoutTransform(data);
    this->gameObject->CascadeUpdate(data);

}

Transform::Transform(): translation(glm::vec3(0)), rotation(glm::quat(1, 0, 0, 0)), scale(glm::vec3(1)), worldMatrix(glm::mat4(1)), modelMatrix(glm::mat4(1))
{
}

Transform::~Transform()
{
}
