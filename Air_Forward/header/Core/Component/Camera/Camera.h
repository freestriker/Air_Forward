#pragma once
#include "Core/Component/Component.h"
#include <glm/mat4x4.hpp>
#include <array>

namespace Core
{
	namespace Component
	{
		namespace Camera
		{
			class Camera: public Core::Component::Component
			{
			protected:
				glm::mat4 _modelMatrix;
				void OnUpdate() override;
				Camera();
				virtual ~Camera();
			public:
				float nearFlat;
				float farFlat;
				float aspectRatio;
				glm::mat4 ViewMatrix();
				virtual glm::mat4 ProjectionMatrix() = 0;
				virtual std::array<glm::vec4, 6> ClipPlanes() = 0;
			};
		}
	}
}