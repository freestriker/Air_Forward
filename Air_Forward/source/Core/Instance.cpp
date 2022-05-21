#include "Core/Instance.h"
#include "Core/Object/GameObject.h"

Core::Instance::RootGameObject Core::Instance::rootObject = Core::Instance::RootGameObject();
std::unordered_set<Core::Object::GameObject*> Core::Instance::_validGameObjectInIteration = std::unordered_set<Core::Object::GameObject*>();
std::unordered_set<Core::Component::Component*> Core::Instance::_validComponentInIteration = std::unordered_set<Core::Component::Component*>();

Core::Instance::Instance()
{
}

Core::Instance::~Instance()
{
}

Core::Instance::RootGameObject::RootGameObject()
	: _gameObject("RootGameObject")
{
}

Core::Instance::RootGameObject::~RootGameObject()
{
}