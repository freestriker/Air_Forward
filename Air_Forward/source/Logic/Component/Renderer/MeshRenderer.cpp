#include "Logic/Component/Renderer/MeshRenderer.h"
#include <rttr/registration>
RTTR_REGISTRATION
{
    using namespace rttr;
    registration::class_<Logic::Component::Renderer::MeshRenderer>("Logic::Component::Renderer::MeshRenderer");
}

Logic::Component::Renderer::MeshRenderer::MeshRenderer()
	: Renderer()
{
}

Logic::Component::Renderer::MeshRenderer::~MeshRenderer()
{
}
