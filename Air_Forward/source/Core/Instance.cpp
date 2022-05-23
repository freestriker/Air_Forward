#include "Core/Instance.h"
#include "Core/Object/GameObject.h"

Core::Instance::RootGameObject Core::Instance::rootObject = Core::Instance::RootGameObject();
Utils::Condition* Core::Instance::_exitCondition = new Utils::Condition();
std::unordered_set<Core::Object::GameObject*> Core::Instance::_validGameObjectInIteration = std::unordered_set<Core::Object::GameObject*>();
std::unordered_set<Core::Component::Component*> Core::Instance::_validComponentInIteration = std::unordered_set<Core::Component::Component*>();

void Core::Instance::Exit()
{
	_exitCondition->Awake();
}

void Core::Instance::WaitExit()
{
	_exitCondition->Wait();
}

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