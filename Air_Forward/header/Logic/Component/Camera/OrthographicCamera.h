#pragma once
#include "Logic/Component/Camera/Camera.h"

namespace Logic
{
	namespace Component
	{
		namespace Camera
		{
			class OrthographicCamera final : public Logic::Component::Camera::Camera
			{
			private:
				glm::vec4 GetParameter()override;
			public:
				float size;
				OrthographicCamera();
				virtual ~OrthographicCamera();
				glm::mat4 ProjectionMatrix() override;
				std::array<glm::vec4, 6> ClipPlanes() override;

				RTTR_ENABLE(Logic::Component::Camera::Camera)
			};
		}
	}
}