#pragma once
#include "Logic/Component/Renderer/Renderer.h"
namespace Logic
{
	namespace Component
	{
		namespace Renderer
		{
			class MeshRenderer : public Logic::Component::Renderer::Renderer
			{
			public:
				MeshRenderer();
				virtual ~MeshRenderer();

				RTTR_ENABLE(Logic::Component::Renderer::Renderer)
			};
		}
	}
}