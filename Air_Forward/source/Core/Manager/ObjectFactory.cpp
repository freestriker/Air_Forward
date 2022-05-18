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
		child = gameObject->Child();
	}
	
	auto itertor = gameObject->_timeSqueueComponentsHead.GetItertor();
	while (itertor.IsValid())
	{
		auto component = static_cast<Core::Component::Component*>(itertor.Node());
		itertor++;

		gameObject->RemoveComponent(component);
		component->OnDestory();
		delete component;
	}

	delete gameObject;
}
