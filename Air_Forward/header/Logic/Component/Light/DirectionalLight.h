#pragma once
#include "Logic/Component/Light/Light.h"

namespace Logic
{
	namespace Component
	{
		namespace Light
		{
			class DirectionalLight final : public Logic::Component::Light::Light
			{
			public:
				LightData GetLightData() override;
				DirectionalLight();
				~DirectionalLight();
			private:
				void OnUpdate()override;

				RTTR_ENABLE(Logic::Component::Light::Light)
			};
		}
	}
}