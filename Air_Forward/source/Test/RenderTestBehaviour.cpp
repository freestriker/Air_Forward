#include "Test/RenderTestBehaviour.h"
#include "Logic/Component/Renderer/MeshRenderer.h"
#include "Logic/Object/GameObject.h"
#include "Logic/Core/Instance.h"
#include <algorithm>

Test::RenderTestBehaviour::RenderTestBehaviour()
	: meshTask()
	, shaderTask()
	, texture2DTask()
	, loaded(false)
	, mesh(nullptr)
	, shader(nullptr)
	, texture2d(nullptr)
	, material(nullptr)
	, rotationSpeed(0.5235987755982988)
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

	auto rotation = GameObject()->transform.Rotation();
	rotation.x = std::fmod(rotation.x + rotationSpeed * 0.3f * Logic::Core::Instance::time.DeltaDuration(), 6.283185307179586);
	rotation.y = std::fmod(rotation.y + rotationSpeed * 0.6f * Logic::Core::Instance::time.DeltaDuration(), 6.283185307179586);
	rotation.z = std::fmod(rotation.z + rotationSpeed * Logic::Core::Instance::time.DeltaDuration(), 6.283185307179586);
	GameObject()->transform.SetRotation(rotation);
}

void Test::RenderTestBehaviour::OnDestroy()
{
}
