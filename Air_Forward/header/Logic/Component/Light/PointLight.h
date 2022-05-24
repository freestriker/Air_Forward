#pragma once
#include "Logic/Component/Light/Light.h"

namespace Logic
{
	namespace Component
	{
		namespace Light
		{
			class PointLight final : public Logic::Component::Light::Light
			{
			public:
				float range;
				LightData GetLightData() override;
				PointLight();
				~PointLight();
			private:
				void OnUpdate()override;

				RTTR_ENABLE(Logic::Component::Light::Light)
			};
		}
	}
}