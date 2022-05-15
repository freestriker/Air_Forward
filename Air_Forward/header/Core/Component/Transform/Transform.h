#pragma once
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <glm/gtc/quaternion.hpp>
#include "Core/Component/Component.h"

namespace Core
{
	namespace Component
	{
		namespace Transform
		{
			class Transform final 
				: public Core::Component::Component
			{
			private:
				bool OnCheckValid()override;
				bool Active();
				void SetActive();
			public:
				glm::quat _rotation;
				glm::vec3 _translation;
				glm::vec3 _scale;

				void SetTranslation(glm::vec3 translation);
				void SetRotation(glm::quat rotation);
				void SetScale(glm::vec3 scale);
				void SetTranslationRotationScale(glm::vec3 translation, glm::quat rotation, glm::vec3 scale);

				glm::mat4 TranslationMatrix();
				glm::mat4 RotationMatrix();
				glm::mat4 ScaleMatrix();

				glm::quat Rotation();
				glm::vec3 Translation();
				glm::vec3 Scale();

				Transform();
				virtual ~Transform();

				RTTR_ENABLE(Core::Component::Component)
			};
		}
	}
}

