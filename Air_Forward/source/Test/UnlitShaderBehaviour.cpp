#include "Test/UnlitShaderBehaviour.h"
#include "Logic/Component/Renderer/MeshRenderer.h"
#include "Logic/Object/GameObject.h"
#include "Logic/Core/Instance.h"
#include <algorithm>
#include <Graphic/Asset/TextureCube.h>
#include <rttr/registration>
RTTR_REGISTRATION
{
	rttr::registration::class_<Test::UnlitShaderBehaviour>("Test::UnlitShaderBehaviour");
}

Test::UnlitShaderBehaviour::UnlitShaderBehaviour(std::string meshPath, std::string shaderPath)
	: meshTask()
	, shaderTask()
	, loaded(false)
	, mesh(nullptr)
	, shader(nullptr)
	, material(nullptr)
	, _meshPath(meshPath)
	, _shaderPath(shaderPath)
{
}

Test::UnlitShaderBehaviour::UnlitShaderBehaviour()
	: UnlitShaderBehaviour("..\\Asset\\Mesh\\DefaultMesh.ply", "..\\Asset\\Shader\\GlassShader.shader")
{
}

Test::UnlitShaderBehaviour::~UnlitShaderBehaviour()
{
}

void Test::UnlitShaderBehaviour::OnAwake()
{
}

void Test::UnlitShaderBehaviour::OnStart()
{
	meshTask = Graphic::Asset::Mesh::LoadAsync(_meshPath.c_str());
	shaderTask = Graphic::Asset::Shader::LoadAsync(_shaderPath.c_str());
}

void Test::UnlitShaderBehaviour::OnUpdate()
{
	if (!loaded && meshTask._Is_ready() && shaderTask._Is_ready())
	{
		mesh = meshTask.get();
		shader = shaderTask.get();
		material = new Graphic::Material(shader);

		loaded = true;

		auto meshRenderer = GameObject()->GetComponent<Logic::Component::Renderer::Renderer>();
		meshRenderer->material = material;
		meshRenderer->mesh = mesh;
	}
}

void Test::UnlitShaderBehaviour::OnDestroy()
{
}
