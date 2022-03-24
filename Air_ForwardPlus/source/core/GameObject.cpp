#include "core/GameObject.h"
#include <cassert>

GameObject::GameObject(): GameObject("New GameObject")
{
}

GameObject::~GameObject()
{
	components.clear();
}

GameObject::GameObject(std::string name): name(name), components()
{
}

void GameObject::UpdateSelf(GameObject* parentGameObject)
{
}

void GameObject::UpdateSelfWithoutTransform(GameObject* parentGameObject)
{
}

void GameObject::CascadeUpdate(GameObject* parentGameObject)
{
}

void GameObject::OnAddedAsChild(GameObject* parentGameObject)
{
}

void GameObject::AddComponent(Component* component)
{
	for each (Component* c in components)
	{
		assert(c == component, "Already contains a same component.");
	}
	components.push_back(component);
}

void GameObject::RemoveComponent(Component* component)
{

}
