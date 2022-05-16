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
