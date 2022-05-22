#pragma once
#pragma once
#include "Core/Component/Camera/Camera.h"
#include <glm/ext/matrix_clip_space.hpp>

namespace Core
{
	namespace Component
	{
		namespace Camera
		{
			class PerspectiveCamera final : public Core::Component::Camera::Camera
			{
			public:
				float fovAngle;
				PerspectiveCamera();
				virtual ~PerspectiveCamera();
				glm::mat4 ProjectionMatrix() override;
				std::array<glm::vec4, 6> ClipPlanes() override;
			};
		}
	}
}