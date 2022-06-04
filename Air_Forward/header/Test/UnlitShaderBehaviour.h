#pragma once
#include "Logic/Component/Behaviour/CppBehaviour.h"
#include <string>
#include "Graphic/Asset/Mesh.h"
#include "Graphic/Asset/Shader.h"
#include "Graphic/Asset/Texture2D.h"
#include "Graphic/Material.h"

namespace Test
{
	class UnlitShaderBehaviour : public Logic::Component::Behaviour::CppBehaviour
	{
	public:
		std::future<Graphic::Asset::Mesh*> meshTask;
		std::future<Graphic::Asset::Shader*> shaderTask;
		bool loaded;
		Graphic::Asset::Mesh* mesh;
		Graphic::Asset::Shader* shader;
		Graphic::Material* material;

		UnlitShaderBehaviour(std::string meshPath, std::string shaderPath);
		UnlitShaderBehaviour();
		~UnlitShaderBehaviour();
		void OnAwake()override;
		void OnStart()override;
		void OnUpdate()override;
		void OnDestroy()override;
	private:
		std::string _meshPath;
		std::string _shaderPath;

		RTTR_ENABLE(Logic::Component::Behaviour::CppBehaviour)
	};
}
