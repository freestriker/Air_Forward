#include "Logic/Component/Renderer/TransparentRenderer.h"
#include <rttr/registration>
RTTR_REGISTRATION
{
    using namespace rttr;
    registration::class_<Logic::Component::Renderer::TransparentRenderer>("Logic::Component::Renderer::TransparentRenderer");
}

Logic::Component::Renderer::TransparentRenderer::TransparentRenderer()
    : Renderer()
{
}

Logic::Component::Renderer::TransparentRenderer::~TransparentRenderer()
{
}
