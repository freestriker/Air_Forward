#include "Core/Component/Renderer/Renderer.h"
#include "Graphic/Instance/Buffer.h"
#include "Core/Object/GameObject.h"
#include "Graphic/Material.h"
#include <Utils/Log.h>


void Core::Component::Renderer::Renderer::OnUpdate()
{
	_modelMatrix = _gameObject->transform.ModelMatrix();
	Utils::Log::Message("Core::Component::Renderer::Renderer::OnUpdate()");
}

Core::Component::Renderer::Renderer::Renderer()
	: Component(ComponentType::RENDERER)
	, _modelMatrix()
	, _matrixBuffer(new Graphic::Instance::Buffer(sizeof(MatrixData), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))
	, mesh(nullptr)
	, material(nullptr)
{
}

Core::Component::Renderer::Renderer::~Renderer()
{
}

void Core::Component::Renderer::Renderer::SetMatrixData(glm::mat4& viewMatrix, glm::mat4& projectionMatrix)
{
	MatrixData data = { _modelMatrix , viewMatrix , projectionMatrix };
	_matrixBuffer->WriteBuffer(&data, sizeof(MatrixData));
	material->SetUniformBuffer("matrixData", _matrixBuffer);
}
