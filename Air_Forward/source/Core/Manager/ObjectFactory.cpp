#include "Core/Manager/ObjectFactory.h"
#include "Core/Component/Component.h"
#include "Core/Object/GameObject.h"

Core::Manager::ObjectFactory::ObjectFactory()
{
}

Core::Manager::ObjectFactory::~ObjectFactory()
{
}

void Core::Manager::ObjectFactory::Destroy(Component::Component* component)
{
	if (component->_gameObject)
	{
		component->_gameObject->RemoveComponent(component);
	}
	component->OnDestory();
	delete component;
}

void Core::Manager::ObjectFactory::Destroy(Object::GameObject* gameObject)
{
	gameObject->RemoveSelf();

	Object::GameObject* child = gameObject->Child();
	while (child)
	{
		Destroy(child);
		child = child->Brother();
	}
	
	while (gameObject->_components.size() > 0)
	{
		auto backComponent = gameObject->_components.back();

		backComponent->_gameObject = nullptr;
		gameObject->_components.pop_back();

		backComponent->OnDestory();
		delete backComponent;
	}

	delete gameObject;
}
