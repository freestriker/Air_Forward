#include "Test/RenderTestBehaviour.h"
#include "Logic/Component/Renderer/MeshRenderer.h"
#include "Logic/Object/GameObject.h"

Test::RenderTestBehaviour::RenderTestBehaviour()
	: meshTask()
	, shaderTask()
	, texture2DTask()
	, loaded(false)
	, mesh(nullptr)
	, shader(nullptr)
	, texture2d(nullptr)
	, material(nullptr)
{
}

Test::RenderTestBehaviour::~RenderTestBehaviour()
{
}

void Test::RenderTestBehaviour::OnAwake()
{
}

void Test::RenderTestBehaviour::OnStart()
{
	meshTask = Graphic::Asset::Mesh::LoadAsync("..\\Asset\\Mesh\\DefaultMesh.ply");
	shaderTask = Graphic::Asset::Shader::LoadAsync("..\\Asset\\Shader\\DefaultShader.shader");
	texture2DTask = Graphic::Asset::Texture2D::LoadAsync("..\\Asset\\Texture\\DefaultTexture.png");
	Utils::Log::Message("Test::RenderTestBehaviour::OnUpdate() start load.");
}

void Test::RenderTestBehaviour::OnUpdate()
{
	if (!loaded && meshTask._Is_ready() && shaderTask._Is_ready() && texture2DTask._Is_ready())
	{
		mesh = meshTask.get();
		shader = shaderTask.get();
		texture2d = texture2DTask.get();
		material = new Graphic::Material(shader);
		material->SetTexture2D("albedo", texture2d);

		loaded = true;
		Utils::Log::Message("Test::RenderTestBehaviour::OnUpdate() finish load.");

		auto meshRenderer = GameObject()->GetComponent<Logic::Component::Renderer::Renderer>();
		meshRenderer->material = material;
		meshRenderer->mesh = mesh;
	}
}

void Test::RenderTestBehaviour::OnDestroy()
{
}
