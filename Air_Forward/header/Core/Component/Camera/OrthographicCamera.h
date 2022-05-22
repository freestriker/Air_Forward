#pragma once
#include "Core/Component/Camera/Camera.h"

namespace Core
{
	namespace Component
	{
		namespace Camera
		{
			class OrthographicCamera final : public Core::Component::Camera::Camera
			{
			public:
				float size;
				OrthographicCamera();
				virtual ~OrthographicCamera();
				glm::mat4 ProjectionMatrix() override;
				std::array<glm::vec4, 6> ClipPlanes() override;

				RTTR_ENABLE(Core::Component::Camera::Camera)
			};
		}
	}
}