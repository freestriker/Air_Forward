#include "Logic/Component/Renderer/BackgroundRenderer.h"
#include <rttr/registration>
#include <Graphic/Core/Window.h>
RTTR_REGISTRATION
{
    using namespace rttr;
    registration::class_<Logic::Component::Renderer::BackgroundRenderer>("Logic::Component::Renderer::BackgroundRenderer");
}

void Logic::Component::Renderer::BackgroundRenderer::OnStart()
{
}

Logic::Component::Renderer::BackgroundRenderer::BackgroundRenderer()
    : Renderer()
{
}

Logic::Component::Renderer::BackgroundRenderer::~BackgroundRenderer()
{
}
