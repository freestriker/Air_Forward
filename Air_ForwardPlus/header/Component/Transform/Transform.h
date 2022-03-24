#pragma once
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <glm/gtc/quaternion.hpp>
#include "core/Component.h"

class Transform : public Component
{
public:
	glm::quat rotation;
	glm::vec3 translation;
	glm::vec3 scale;
	glm::mat4 modelMatrix;
	glm::mat4 worldMatrix;

	void SetTranslation(glm::vec3 translation);
	void SetRotation(glm::quat rotation);
	void SetScale(glm::vec3 scale);
	void SetTranslationRotationScale(glm::vec3 translation, glm::quat rotation, glm::vec3 scale);
	void UpdateSelf(GameObject* parentGameObject) override;
	void UpdateGameObject(GameObject* parentGameObject);

	Transform();
	virtual ~Transform();
	RTTR_ENABLE(Component)
};

