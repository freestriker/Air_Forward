#pragma once
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <glm/gtc/quaternion.hpp>
#include "Logic/Component/Component.h"

namespace Logic
{
	namespace Object
	{
		class GameObject;
	}
	namespace Component
	{
		namespace Transform
		{
			class Transform final 
				: public Logic::Component::Component
			{
				friend class Logic::Object::GameObject;
			private:
				bool Active();
				void SetActive();

				void UpdateModelMatrix(glm::mat4& parentModelMatrix);
				glm::vec3 _rotation;
				glm::vec3 _translation;
				glm::vec3 _scale;

				glm::mat4 _modelMatrix;
				glm::mat4 _relativeModelMatrix;

			public:
				void SetTranslation(glm::vec3 translation);
				void SetRotation(glm::vec3 rotation);
				void SetScale(glm::vec3 scale);
				void SetTranslationRotationScale(glm::vec3 translation, glm::vec3 rotation, glm::vec3 scale);

				glm::mat4 TranslationMatrix();
				glm::mat4 RotationMatrix();
				glm::mat4 ScaleMatrix();
				glm::mat4 ModelMatrix();

				glm::vec3 Rotation();
				glm::vec3 Translation();
				glm::vec3 Scale();

				Transform();
				virtual ~Transform();

				RTTR_ENABLE(Logic::Component::Component)
			};
		}
	}
}

