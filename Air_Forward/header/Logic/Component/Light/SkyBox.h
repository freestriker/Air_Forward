#pragma once
#pragma once
#include "Logic/Component/Light/Light.h"
#include <string>

namespace Graphic
{
	namespace Instance
	{
		class Buffer;
	}
	namespace Asset
	{
		class TextureCube;
	}
}
namespace Logic
{
	namespace Component
	{
		namespace Light
		{
			class SkyBox final : public Logic::Component::Light::Light
			{
			public:
				std::string skyBoxName;
				LightData GetLightData() override;
				Graphic::Asset::TextureCube* TextureCube();
				SkyBox();
				~SkyBox();
			private:
				Graphic::Asset::TextureCube* _textureCube;
				void OnStart()override;
				void OnUpdate()override;

				RTTR_ENABLE(Logic::Component::Light::Light)
			};
		}
	}
}