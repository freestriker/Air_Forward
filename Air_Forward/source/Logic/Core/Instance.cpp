#include "Logic/Core/Instance.h"
#include "Logic/Object/GameObject.h"

Logic::Core::Instance::RootGameObject Logic::Core::Instance::rootObject = Logic::Core::Instance::RootGameObject();
Utils::Condition* Logic::Core::Instance::_exitCondition = new Utils::Condition();
std::unordered_set<Logic::Object::GameObject*> Logic::Core::Instance::_validGameObjectInIteration = std::unordered_set<Logic::Object::GameObject*>();
std::unordered_set<Logic::Component::Component*> Logic::Core::Instance::_validComponentInIteration = std::unordered_set<Logic::Component::Component*>();
Logic::Core::Instance::Time Logic::Core::Instance::time = Logic::Core::Instance::Time();

void Logic::Core::Instance::Exit()
{
	_exitCondition->Awake();
}

void Logic::Core::Instance::WaitExit()
{
	_exitCondition->Wait();
}

Logic::Core::Instance::Instance()
{
}

Logic::Core::Instance::~Instance()
{
}

Logic::Core::Instance::RootGameObject::RootGameObject()
	: _gameObject("RootGameObject")
{
}

Logic::Core::Instance::RootGameObject::~RootGameObject()
{
}

Logic::Core::Instance::Time::Time()
	: _time()
{
}

Logic::Core::Instance::Time::~Time()
{
}