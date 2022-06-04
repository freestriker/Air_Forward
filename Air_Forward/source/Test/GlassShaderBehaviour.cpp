#include "Test/GlassShaderBehaviour.h"
#include "Logic/Component/Renderer/MeshRenderer.h"
#include "Logic/Object/GameObject.h"
#include "Logic/Core/Instance.h"
#include <algorithm>
#include <Graphic/Asset/TextureCube.h>
#include <rttr/registration>
RTTR_REGISTRATION
{
	rttr::registration::class_<Test::GlassShaderBehaviour>("Test::GlassShaderBehaviour");
}

Test::GlassShaderBehaviour::GlassShaderBehaviour()
	: meshTask()
	, shaderTask()
	, loaded(false)
	, mesh(nullptr)
	, shader(nullptr)
	, material(nullptr)
	, rotationSpeed(0.5235987755982988)
{
}

Test::GlassShaderBehaviour::~GlassShaderBehaviour()
{
}

void Test::GlassShaderBehaviour::OnAwake()
{
}

void Test::GlassShaderBehaviour::OnStart()
{
	meshTask = Graphic::Asset::Mesh::LoadAsync("..\\Asset\\Mesh\\DefaultMesh.ply");
	shaderTask = Graphic::Asset::Shader::LoadAsync("..\\Asset\\Shader\\GlassShader.shader");
}

void Test::GlassShaderBehaviour::OnUpdate()
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

	auto rotation = GameObject()->transform.Rotation();
	rotation.x = std::fmod(rotation.x + rotationSpeed * 0.3f * Logic::Core::Instance::time.DeltaDuration(), 6.283185307179586);
	rotation.y = std::fmod(rotation.y + rotationSpeed * 0.6f * Logic::Core::Instance::time.DeltaDuration(), 6.283185307179586);
	rotation.z = std::fmod(rotation.z + rotationSpeed * Logic::Core::Instance::time.DeltaDuration(), 6.283185307179586);
	GameObject()->transform.SetRotation(rotation);
}

void Test::GlassShaderBehaviour::OnDestroy()
{
}
