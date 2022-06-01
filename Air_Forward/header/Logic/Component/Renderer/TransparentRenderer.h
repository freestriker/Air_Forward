#pragma once
#pragma once
#include "Logic/Component/Renderer/Renderer.h"
#include <Graphic/Instance/Image.h>
#include <Graphic/Instance/ImageSampler.h>
namespace Logic
{
	namespace Component
	{
		namespace Renderer
		{
			class TransparentRenderer : public Logic::Component::Renderer::Renderer
			{
			public:
				TransparentRenderer();
				virtual ~TransparentRenderer();

				RTTR_ENABLE(Logic::Component::Renderer::Renderer)
			};
		}
	}
}