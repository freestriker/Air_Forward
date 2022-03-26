#include "core/ObjectFactory.h"
#include "core/GameObject.h"

GameObject* ObjectFactory::InstantiateGameObject()
{
	GameObject* go = new GameObject();
	go->OnAwake();
	go->OnEnable();
	return go;
}

void ObjectFactory::DestoryGameObject(GameObject* gameObject)
{
}

void ObjectFactory::DestoryComponent(Component* component)
{
}
