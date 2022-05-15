#include "Core/Instance.h"
#include "Core/Object/GameObject.h"

Core::Instance::RootGameObject Core::Instance::rootObject = Core::Instance::RootGameObject();


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