#include "core/GameObject.h"
#include <cassert>
#include <rttr/registration>
RTTR_REGISTRATION
{
	using namespace rttr;
	registration::class_<GameObject>("GameObject")
		.constructor<>()
		.method("GetComponent", &GameObject::GetComponent)
		.method("GetComponents", &GameObject::GetComponents)
		.method("AddComponent", &GameObject::AddComponent)
		.method("RemoveComponent", select_overload<void(Component*)>(&GameObject::RemoveComponent))
		.method("RemoveComponent", select_overload<Component*(std::string)>(&GameObject::RemoveComponent))
		.method("RemoveComponents", &GameObject::RemoveComponents)
		.method("AddChild", &GameObject::AddChild)
		.method("RemoveChild", &GameObject::RemoveChild)
		;
}


GameObject::GameObject(): GameObject("New GameObject")
{
}

GameObject::~GameObject()
{
	components.clear();
	delete chain;
}

GameObject::GameObject(std::string name): name(name), components(), transform(), chain(nullptr)
{
}

void GameObject::UpdateSelf(GameObject* parentGameObject)
{
	transform.UpdateSelf(parentGameObject);
	for each (Component* component in components)
	{
		component->UpdateSelf(parentGameObject);

	}
}

void GameObject::UpdateSelfWithoutTransform(GameObject* parentGameObject)
{
	for each (Component * component in components)
	{
		component->UpdateSelf(parentGameObject);

	}
}

void GameObject::CascadeUpdate(GameObject* parentGameObject)
{
	for (ChildBrotherTree<GameObject>::ChildIterator iter = chain->GetChildIterator(); iter.IsValid(); ++iter)
	{
		iter.Node()->object->UpdateSelf(this);
	}
}

void GameObject::AddComponent(Component* component)
{
	for each (Component* c in components)
	{
		assert(c == component && "Already contains a same component.");
	}
	components.push_back(component);
}

void GameObject::RemoveComponent(Component* component)
{
	for (auto iter = components.begin(), end = components.end(); iter < end; iter++)
	{
		if (*iter == component)
		{
			components.erase(iter);
		}
	}
	assert(true && "Do not contains a same component.");
}
Component* GameObject::GetComponent(std::string typeName)
{
	using namespace rttr;
	type class_type = type::get_by_name(typeName);
	type class_component = type::get<Component*>();

	if (class_type)
	{
		if (!class_component.is_derived_from(class_component))
		{
			assert("It is not a Component.");
		}
		for (auto iter = components.begin(), end = components.end(); iter != end; iter++)
		{
			type class_target = type::get(*iter);
			if (class_target == class_type || class_type.is_base_of(class_target))
			{
				return *iter;
			}
		}
	}
	else
	{
		assert(false && "Do not have type.");
	}
}

std::vector<Component*> GameObject::GetComponents(std::string typeName)
{
	using namespace rttr;
	type class_type = type::get_by_name(typeName);
	type class_component = type::get<Component*>();
	std::vector<Component*> removeVector = std::vector<Component*>();

	if (class_type)
	{
		if (!class_component.is_derived_from(class_component))
		{
			assert("It is not a Component.");
		}
		for (auto iter = components.begin(), end = components.end(); iter != end; iter++)
		{
			type class_target = type::get(*iter);
			if (class_target == class_type || class_type.is_base_of(class_target))
			{
				removeVector.push_back(*iter);
			}
		}
		return removeVector;
	}
	else
	{
		assert(false && "Do not have type.");
	}
}

Component* GameObject::RemoveComponent(std::string typeName)
{
	using namespace rttr;
	type class_type = type::get_by_name(typeName);
	type class_component = type::get<Component*>();

	if (class_type)
	{
		if (!class_component.is_derived_from(class_component))
		{
			assert("It is not a Component.");
		}
		for (auto iter = components.begin(), end = components.end(); iter != end; iter++)
		{
			type class_target = type::get(*iter);
			if (class_target == class_type || class_type.is_base_of(class_target))
			{
				Component* t = *iter;
				components.erase(iter);
				return t;
			}
		}
	}
	else
	{
		assert(false && "Do not have type.");
	}
}

std::vector<Component*> GameObject::RemoveComponents(std::string typeName)
{
	using namespace rttr;
	type class_type = type::get_by_name(typeName);
	type class_component = type::get<Component*>();
	std::vector<Component*> removeVector = std::vector<Component*>();

	if (class_type)
	{
		if (!class_component.is_derived_from(class_component))
		{
			assert("It is not a Component.");
		}
		for (auto iter = components.begin(); iter != components.end(); )
		{
			type class_target = type::get(*iter);
			if (class_target == class_type || class_type.is_base_of(class_target))
			{
				removeVector.push_back(*iter);
				iter = components.erase(iter);
			}
			else
			{
				++iter;
			}
		}
		return removeVector;
	}
	else
	{
		assert(false && "Do not have type.");
	}
}
void GameObject::AddChild(GameObject* child)
{
	auto chain = new ChildBrotherTree<GameObject>(child);
	child->chain = chain;

	this->chain->AddChild(chain);

	UpdateSelf(this);
	CascadeUpdate(this);

}

void GameObject::RemoveChild(GameObject* child)
{
	delete child->chain->Remove();
	child->chain = nullptr;
}
