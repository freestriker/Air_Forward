#include "Test/BackgroundRendererBehaviour.h"
#include "Logic/Component/Renderer/MeshRenderer.h"
#include "Logic/Component/Renderer/BackgroundRenderer.h"
#include "Logic/Object/GameObject.h"
#include "Logic/Core/Instance.h"
#include <algorithm>
#include <Graphic/Asset/TextureCube.h>
#include <rttr/registration>
RTTR_REGISTRATION
{
	rttr::registration::class_<Test::BackgroundRendererBehaviour>("Test::BackgroundRendererBehaviour");
}

Test::BackgroundRendererBehaviour::BackgroundRendererBehaviour()
	: meshTask()
	, shaderTask()
	, textureCubeTask()
	, loaded(false)
	, mesh(nullptr)
	, shader(nullptr)
	, textureCube(nullptr)
	, material(nullptr)
	, rotationSpeed(0.5235987755982988)
{
}

Test::BackgroundRendererBehaviour::~BackgroundRendererBehaviour()
{
}

void Test::BackgroundRendererBehaviour::OnAwake()
{
}

void Test::BackgroundRendererBehaviour::OnStart()
{
	meshTask = Graphic::Asset::Mesh::LoadAsync("..\\Asset\\Mesh\\SkyBoxMesh.ply");
	shaderTask = Graphic::Asset::Shader::LoadAsync("..\\Asset\\Shader\\DrawSkyBoxShader.shader");
	textureCubeTask = Graphic::Asset::TextureCube::LoadAsync("..\\Asset\\Texture\\DefaultTextureCube.json");
}

void Test::BackgroundRendererBehaviour::OnUpdate()
{
	if (!loaded && meshTask._Is_ready() && shaderTask._Is_ready() && textureCubeTask._Is_ready())
	{
		auto meshRenderer = GameObject()->GetComponent<Logic::Component::Renderer::BackgroundRenderer>();
		
		mesh = meshTask.get();
		shader = shaderTask.get();
		textureCube= textureCubeTask.get();
		material = new Graphic::Material(shader);
		material->SetTextureCube("backgroundTexture", textureCube);
		material->SetSlotData("depthTexture", { 0 }, { {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, meshRenderer->_temporaryImageSampler->VkSampler_(), meshRenderer->_temporaryImage->VkImageView_(), VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL} });

		loaded = true;

		meshRenderer->material = material;
		meshRenderer->mesh = mesh;
	}
}

void Test::BackgroundRendererBehaviour::OnDestroy()
{
}
