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
	component->OnDestroy();
	if (component->_gameObject) component->_gameObject->RemoveComponent(component);
	delete component;
}

void Core::Manager::ObjectFactory::Destroy(Object::GameObject* gameObject)
{
	Object::GameObject* child = gameObject->Child();
	while (child)
	{
		Destroy(child);
		child = gameObject->Child();
	}
	
	for (auto iterator = gameObject->_timeSqueueComponentsHead.GetIterator(); iterator.IsValid(); iterator = gameObject->_timeSqueueComponentsHead.GetIterator())
	{
		Destroy(static_cast<Core::Component::Component*>(iterator.Node()));
	}

	gameObject->RemoveSelf();
	delete gameObject;
}
