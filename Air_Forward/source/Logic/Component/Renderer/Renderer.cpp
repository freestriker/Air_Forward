#include "Logic/Component/Renderer/Renderer.h"
#include "Graphic/Instance/Buffer.h"
#include "Logic/Object/GameObject.h"
#include "Graphic/Material.h"
#include <Utils/Log.h>
#include <rttr/registration>
RTTR_REGISTRATION
{
    using namespace rttr;
    registration::class_<Logic::Component::Renderer::Renderer>("Logic::Component::Renderer::Renderer");
}

void Logic::Component::Renderer::Renderer::OnUpdate()
{
	_modelMatrix = _gameObject->transform.ModelMatrix();
}

Logic::Component::Renderer::Renderer::Renderer()
	: Component(ComponentType::RENDERER)
	, _modelMatrix()
	, _matrixBuffer(new Graphic::Instance::Buffer(sizeof(MatrixData), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))
	, mesh(nullptr)
	, material(nullptr)
{
}

Logic::Component::Renderer::Renderer::~Renderer()
{
}

void Logic::Component::Renderer::Renderer::SetMatrixData(glm::mat4& viewMatrix, glm::mat4& projectionMatrix)
{
	MatrixData data = { _modelMatrix , viewMatrix , projectionMatrix, glm::transpose(glm::inverse(_modelMatrix))};
	_matrixBuffer->WriteBuffer(&data, sizeof(MatrixData));
	material->SetUniformBuffer("matrixData", _matrixBuffer);
}

const glm::mat4& Logic::Component::Renderer::Renderer::ModelMatrix()
{
	return _modelMatrix;
}
