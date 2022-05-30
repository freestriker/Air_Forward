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
			class BackgroundRenderer : public Logic::Component::Renderer::Renderer
			{
			private:
				void OnStart()override;
			public:
				BackgroundRenderer();
				virtual ~BackgroundRenderer();
				Graphic::Instance::Image* _temporaryImage;
				Graphic::Instance::ImageSampler* _temporaryImageSampler;

				RTTR_ENABLE(Logic::Component::Renderer::Renderer)
			};
		}
	}
}