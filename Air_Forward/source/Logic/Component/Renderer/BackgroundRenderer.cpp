#include "Logic/Component/Renderer/BackgroundRenderer.h"
#include <rttr/registration>
RTTR_REGISTRATION
{
    using namespace rttr;
    registration::class_<Logic::Component::Renderer::BackgroundRenderer>("Logic::Component::Renderer::BackgroundRenderer");
}

Logic::Component::Renderer::BackgroundRenderer::BackgroundRenderer()
    : Renderer()
{
    enableFrustumCulling = false;
}

Logic::Component::Renderer::BackgroundRenderer::~BackgroundRenderer()
{
}
