#pragma once
#pragma once
#include "Logic/Component/Camera/Camera.h"
#include <glm/ext/matrix_clip_space.hpp>

namespace Logic
{
	namespace Component
	{
		namespace Camera
		{
			class PerspectiveCamera final : public Logic::Component::Camera::Camera
			{
			private:
				glm::vec4 GetParameter()override;
			public:
				float fovAngle;
				PerspectiveCamera();
				virtual ~PerspectiveCamera();
				glm::mat4 ProjectionMatrix() override;
				std::array<glm::vec4, 6> ClipPlanes() override;

				RTTR_ENABLE(Logic::Component::Camera::Camera)
			};
		}
	}
}