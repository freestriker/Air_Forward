#pragma once
#include "Core/Component/Renderer/Renderer.h"
namespace Core
{
	namespace Component
	{
		namespace Renderer
		{
			class MeshRenderer : public Core::Component::Renderer::Renderer
			{
			public:
				MeshRenderer();
				virtual ~MeshRenderer();

				RTTR_ENABLE(Core::Component::Renderer::Renderer)
			};
		}
	}
}