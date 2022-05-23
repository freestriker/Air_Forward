#include "Logic/Manager/ObjectFactory.h"
#include "Logic/Component/Component.h"
#include "Logic/Object/GameObject.h"

Logic::Manager::ObjectFactory::ObjectFactory()
{
}

Logic::Manager::ObjectFactory::~ObjectFactory()
{
}

void Logic::Manager::ObjectFactory::Destroy(Component::Component* component)
{
	component->OnDestroy();
	if (component->_gameObject) component->_gameObject->RemoveComponent(component);
	delete component;
}

void Logic::Manager::ObjectFactory::Destroy(Object::GameObject* gameObject)
{
	Object::GameObject* child = gameObject->Child();
	while (child)
	{
		Destroy(child);
		child = gameObject->Child();
	}
	
	for (auto iterator = gameObject->_timeSqueueComponentsHead.GetIterator(); iterator.IsValid(); iterator = gameObject->_timeSqueueComponentsHead.GetIterator())
	{
		Destroy(static_cast<Logic::Component::Component*>(iterator.Node()));
	}

	gameObject->RemoveSelf();
	delete gameObject;
}
