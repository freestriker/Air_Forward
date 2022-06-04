#include "Test/MirrorShaderBehaviour.h"
#include "Logic/Component/Renderer/MeshRenderer.h"
#include "Logic/Object/GameObject.h"
#include "Logic/Core/Instance.h"
#include <algorithm>
#include <Graphic/Asset/TextureCube.h>
#include <rttr/registration>
RTTR_REGISTRATION
{
	rttr::registration::class_<Test::MirrorShaderBehaviour>("Test::MirrorShaderBehaviour");
}

Test::MirrorShaderBehaviour::MirrorShaderBehaviour()
	: meshTask()
	, shaderTask()
	, loaded(false)
	, mesh(nullptr)
	, shader(nullptr)
	, material(nullptr)
{
}

Test::MirrorShaderBehaviour::~MirrorShaderBehaviour()
{
}

void Test::MirrorShaderBehaviour::OnAwake()
{
}

void Test::MirrorShaderBehaviour::OnStart()
{
	meshTask = Graphic::Asset::Mesh::LoadAsync("..\\Asset\\Mesh\\DefaultMesh.ply");
	shaderTask = Graphic::Asset::Shader::LoadAsync("..\\Asset\\Shader\\MirrorShader.shader");
}

void Test::MirrorShaderBehaviour::OnUpdate()
{
	if (!loaded && meshTask._Is_ready() && shaderTask._Is_ready())
	{
		auto meshRenderer = GameObject()->GetComponent<Logic::Component::Renderer::MeshRenderer>();

		mesh = meshTask.get();
		shader = shaderTask.get();
		material = new Graphic::Material(shader);

		loaded = true;

		meshRenderer->material = material;
		meshRenderer->mesh = mesh;
	}
}

void Test::MirrorShaderBehaviour::OnDestroy()
{
}
