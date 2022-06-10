#include "Test/TransparentRendererBehaviour.h"
#include "Logic/Component/Renderer/TransparentRenderer.h"
#include "Logic/Object/GameObject.h"
#include "Logic/Core/Instance.h"
#include <algorithm>
#include <Graphic/Asset/TextureCube.h>
#include <rttr/registration>
RTTR_REGISTRATION
{
	rttr::registration::class_<Test::TransparentRendererBehaviour>("Test::TransparentRendererBehaviour");
}

Test::TransparentRendererBehaviour::TransparentRendererBehaviour()
	: meshTask()
	, shaderTask()
	, texture2DTask()
	, loaded(false)
	, mesh(nullptr)
	, shader(nullptr)
	, texture2D(nullptr)
	, material(nullptr)
{
}

Test::TransparentRendererBehaviour::~TransparentRendererBehaviour()
{
}

void Test::TransparentRendererBehaviour::OnAwake()
{
}

void Test::TransparentRendererBehaviour::OnStart()
{
	meshTask = Graphic::Asset::Mesh::LoadAsync("..\\Asset\\Mesh\\SquareMesh.ply");
	shaderTask = Graphic::Asset::Shader::LoadAsync("..\\Asset\\Shader\\DefaultTransparentShader.shader");
	texture2DTask = Graphic::Asset::Texture2D::LoadAsync("..\\Asset\\Texture\\BrokenGlassTexture2d .json");
}

void Test::TransparentRendererBehaviour::OnUpdate()
{
	if (!loaded && meshTask._Is_ready() && shaderTask._Is_ready() && texture2DTask._Is_ready())
	{
		auto meshRenderer = GameObject()->GetComponent<Logic::Component::Renderer::TransparentRenderer>();

		mesh = meshTask.get();
		shader = shaderTask.get();
		texture2D = texture2DTask.get();
		material = new Graphic::Material(shader);
		material->SetTexture2D("albedo", texture2D);

		loaded = true;

		meshRenderer->material = material;
		meshRenderer->mesh = mesh;
	}
}

void Test::TransparentRendererBehaviour::OnDestroy()
{
}
