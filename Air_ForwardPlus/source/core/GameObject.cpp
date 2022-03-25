#include "core/GameObject.h"
#include "core/Component.h"
#include <cassert>
#include <rttr/registration>

RTTR_REGISTRATION
{
	using namespace rttr;
	registration::class_<GameObject>("GameObject")
		.method("Active", &GameObject::Active)
		.method("SetActive", &GameObject::SetActive)
		.method("GetComponent", &GameObject::GetComponent)
		.method("GetComponents", &GameObject::GetComponents)
		.method("AddComponent", &GameObject::AddComponent)
		.method("RemoveComponent", select_overload<void(Component*)>(&GameObject::RemoveComponent))
		.method("RemoveComponent", select_overload<Component*(std::string)>(&GameObject::RemoveComponent))
		.method("RemoveComponents", &GameObject::RemoveComponents)
		.method("Parent", &GameObject::Parent)
		.method("Child", &GameObject::Child)
		.method("Brother", &GameObject::Brother)
		.method("AddChild", &GameObject::AddChild)
		.method("RemoveChild", &GameObject::RemoveChild)
		.method("RemoveSelf", &GameObject::RemoveSelf)
		;
}


GameObject::GameObject(): GameObject("New GameObject")
{
}

GameObject::~GameObject()
{
	components.clear();
}

GameObject::GameObject(std::string name):LifeTime(), name(name), components(), transform(), chain(), active(true)
{
	chain.object = this;
}

void GameObject::SetActive(bool active)
{
	if (this->active != active)
	{
		this->active = active;
		if (active)
		{
			OnEnable();
		}
		else
		{
			OnDisable();
		}
	}
}

bool GameObject::Active()
{
	return active;
}

void GameObject::AddComponent(Component* component)
{
	components.push_back(component);
	
	component->gameObject = this;
	component->OnStart();
	if (active)
	{
		if (component->active)
		{
			component->OnEnable();
		}
	}
}

void GameObject::RemoveComponent(Component* component)
{
	for (auto iter = components.begin(), end = components.end(); iter < end; iter++)
	{
		Component* c = *iter;
		if (c == component)
		{
			components.erase(iter);

			c->gameObject = nullptr;
			if (active && component->active)
			{
				component->OnDisable();
			}

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
			Component* t = *iter;
			type class_target = type::get(t);
			if (class_target == class_type || class_type.is_base_of(class_target))
			{
				components.erase(iter);

				t->gameObject = nullptr;
				if (active && t->active)
				{
					t->OnDisable();
				}

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
			Component* c = *iter;
			type class_target = type::get(c);
			if (class_target == class_type || class_type.is_base_of(class_target))
			{
				iter = components.erase(iter);

				c->gameObject = nullptr;
				if (active && c->active)
				{
					c->OnDisable();
				}

				removeVector.push_back(c);
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

GameObject* GameObject::Parent()
{
	return chain.parent ? chain.parent->object : nullptr;
}
GameObject* GameObject::Child()
{
	return chain.child ? chain.child->object : nullptr;
}
GameObject* GameObject::Brother()
{
	return chain.brother ? chain.brother->object : nullptr;
}
void GameObject::AddChild(GameObject* child)
{
	this->chain.AddChild(&child->chain);
}

void GameObject::RemoveChild(GameObject* child)
{
	if (child->Parent() == this)
	{
		child->RemoveSelf();
	}
}

void GameObject::RemoveSelf()
{
	chain.Remove();
}


void GameObject::OnAwake()
{
}

void GameObject::OnEnable()
{
	for each (Component * component in components)
	{
		if (component->active)
		{
			component->OnEnable();
		}
	}
	for (ChildBrotherTree<GameObject>::ChildIterator iter = chain.GetChildIterator(); iter.IsValid(); ++iter)
	{
		GameObject* go = iter.Node()->object;
		if (go->active)
		{
			go->OnEnable();
		}
	}
}

void GameObject::OnDisable()
{
	for each (Component * component in components)
	{
		if (component->active)
		{
			component->OnDisable();
		}
	}
	for (ChildBrotherTree<GameObject>::ChildIterator iter = chain.GetChildIterator(); iter.IsValid(); ++iter)
	{
		GameObject* go = iter.Node()->object;
		if (go->active)
		{
			go->OnDisable();
		}
	}
}

void GameObject::OnDestory()
{
}
