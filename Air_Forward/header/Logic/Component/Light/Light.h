#pragma once
#include <glm/glm.hpp>
#include "Logic/Component/Component.h"

namespace Logic
{
	namespace Component
	{
		namespace Light
		{
			class Light: public Logic::Component::Component
			{
			public:
				struct LightData
				{
					alignas(16) glm::vec4 type;
					alignas(16) glm::vec3 position;
					alignas(16) glm::vec4 color;
				};
				enum class LightType
				{
					DIRECTIONAL = 1,
					POINT = 2
				};
				virtual LightData GetLightData() = 0;
				glm::vec4 color;
				float intensity;
				const LightType lightType;
			protected:
				glm::vec3 _worldPosition;
				Light(LightType lightType);
				~Light();
				virtual void OnUpdate()override = 0;

				RTTR_ENABLE(Logic::Component::Component)
			};
		}
	}
}