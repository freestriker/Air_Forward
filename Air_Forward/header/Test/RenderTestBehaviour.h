#pragma once
#include "Logic/Component/Behaviour/CppBehaviour.h"
#include <string>
#include "Graphic/Asset/Mesh.h"
#include "Graphic/Asset/Shader.h"
#include "Graphic/Asset/Texture2D.h"
#include "Graphic/Material.h"

namespace Test
{
	class RenderTestBehaviour : public Logic::Component::Behaviour::CppBehaviour
	{
	public:
		std::future<Graphic::Asset::Mesh*> meshTask;
		std::future<Graphic::Asset::Shader*> shaderTask;
		std::future<Graphic::Asset::Texture2D*> texture2DTask;
		bool loaded;
		Graphic::Asset::Mesh* mesh;
		Graphic::Asset::Shader* shader;
		Graphic::Asset::Texture2D* texture2d;
		Graphic::Material* material;

		RenderTestBehaviour();
		~RenderTestBehaviour();
		void OnAwake()override;
		void OnStart()override;
		void OnUpdate()override;
		void OnDestroy()override;
		RTTR_ENABLE(Logic::Component::Behaviour::CppBehaviour)
	};
}