#include "Core/Object/GameObject.h"
#include "Core/Component/Component.h"
#include <cassert>
#include <rttr/registration>
#include "Utils/Log.h"

RTTR_REGISTRATION
{
	using namespace rttr;
	registration::class_<Core::Object::GameObject>("Core::Object::GameObject")
		.method("AddComponent", &Core::Object::GameObject::AddComponent)
		.method("RemoveComponent", &Core::Object::GameObject::_RemoveComponent1)
		.method("RemoveComponentByTypeName", &Core::Object::GameObject::_RemoveComponent2)
		.method("RemoveComponents", &Core::Object::GameObject::_RemoveComponents1)
		.method("RemoveComponentsByTypeName", &Core::Object::GameObject::_RemoveComponents2)
		.method("GetComponent", &Core::Object::GameObject::_GetComponent)
		.method("GetComponents", &Core::Object::GameObject::_GetComponents)
		.method("Parent", &Core::Object::GameObject::Parent)
		.method("Child", &Core::Object::GameObject::Child)
		.method("Brother", &Core::Object::GameObject::Brother)
		.method("AddChild", &Core::Object::GameObject::AddChild)
		.method("RemoveChild", &Core::Object::GameObject::RemoveChild)
		.method("RemoveSelf", &Core::Object::GameObject::RemoveSelf)
		;
}

Core::Object::GameObject::GameObject(std::string name)
	: Utils::ActivableBase()
	, Object()
	, name(name)
	, _timeSqueueComponentsHead()
	, _typeSqueueComponentsHeadMap()
	, transform()
	, _chain()
{
	_chain.SetObject(this);
}

Core::Object::GameObject::GameObject()
	: GameObject("New GameObject")
{
}

Core::Object::GameObject::~GameObject()
{

}

void Core::Object::GameObject::AddComponent(Core::Component::Component* targetComponent)
{
	_timeSqueueComponentsHead.Add(targetComponent);
	if (!_typeSqueueComponentsHeadMap.count(targetComponent->_type))
	{
		_typeSqueueComponentsHeadMap[targetComponent->_type] = std::unique_ptr<Utils::CrossLinkableRowHead>(new Utils::CrossLinkableRowHead());
	}
	_typeSqueueComponentsHeadMap[targetComponent->_type]->Add(targetComponent);
	targetComponent->_gameObject = this;
}

void Core::Object::GameObject::RemoveComponent(Core::Component::Component* targetComponent)
{
	if (targetComponent->_gameObject != this)
	{
		Utils::Log::Exception("Component do not blong to this GameObject.");
	}

	_timeSqueueComponentsHead.Remove(targetComponent);
	_typeSqueueComponentsHeadMap[targetComponent->_type]->Remove(targetComponent);
	targetComponent->_gameObject = nullptr;

	if (!_typeSqueueComponentsHeadMap[targetComponent->_type]->HaveNode())
	{
		_typeSqueueComponentsHeadMap.erase(targetComponent->_type);
	}
}

Core::Component::Component* Core::Object::GameObject::RemoveComponent(std::string targetTypeName)
{
	return RemoveComponent(rttr::type::get_by_name(targetTypeName));
}

void Core::Object::GameObject::_RemoveComponent1(Core::Component::Component* component)
{
	RemoveComponent(component);
}

Core::Component::Component* Core::Object::GameObject::_RemoveComponent2(std::string typeName)
{
	return RemoveComponent(typeName);
}

Core::Component::Component* Core::Object::GameObject::RemoveComponent(rttr::type targetType)
{
	if (!targetType)
	{
		Utils::Log::Exception("Do not have " + targetType.get_name().to_string() + ".");
	}

	if (!targetType.is_derived_from(Core::Component::Component::COMPONENT_TYPE))
	{
		Utils::Log::Exception(targetType.get_name().to_string() + " is not a component.");
	}

	for (const auto& pair : Core::Component::Component::TYPE_MAP)
	{
		if ((targetType == pair.first || targetType.is_base_of(pair.first)) && _typeSqueueComponentsHeadMap.count(pair.second))
		{
			auto found = static_cast<Core::Component::Component*>(_typeSqueueComponentsHeadMap[pair.second]->GetIterator().Node());
			RemoveComponent(found);
			return found;
		}
	}

	Utils::Log::Exception("GameObject " + name + " do not have a " + targetType.get_name().to_string() + " Component.");
}

void Core::Object::GameObject::RemoveComponents(std::vector<Component::Component*> components)
{
	for (const auto& component : components)
	{
		RemoveComponent(component);
	}
}

std::vector<Core::Component::Component*> Core::Object::GameObject::RemoveComponents(std::string targetTypeName)
{
	return RemoveComponents(rttr::type::get_by_name(targetTypeName));
}

void Core::Object::GameObject::_RemoveComponents1(std::vector<Component::Component*> components)
{
	RemoveComponents(components);
}

std::vector<Core::Component::Component*> Core::Object::GameObject::_RemoveComponents2(std::string typeName)
{
	return RemoveComponents(typeName);
}

std::vector<Core::Component::Component*> Core::Object::GameObject::RemoveComponents(rttr::type targetType)
{
	if (!targetType)
	{
		Utils::Log::Exception("Do not have " + targetType.get_name().to_string() + ".");
	}

	if (!targetType.is_derived_from(Core::Component::Component::COMPONENT_TYPE))
	{
		Utils::Log::Exception(targetType.get_name().to_string() + " is not a component.");
	}

	for (const auto& pair : Core::Component::Component::TYPE_MAP)
	{
		if ((targetType == pair.first || targetType.is_base_of(pair.first)) && _typeSqueueComponentsHeadMap.count(pair.second))
		{
			auto targetComponents = std::vector<Core::Component::Component*>();
			auto itertor = _typeSqueueComponentsHeadMap[pair.second]->GetIterator();
			while (itertor.IsValid())
			{
				auto foundComponent = static_cast<Component::Component*>(itertor.Node());
				itertor++;

				_timeSqueueComponentsHead.Remove(foundComponent);
				_typeSqueueComponentsHeadMap[pair.second]->Remove(foundComponent);
				foundComponent->_gameObject = nullptr;

				targetComponents.emplace_back(foundComponent);
			}
			if (!_typeSqueueComponentsHeadMap[pair.second]->HaveNode())
			{
				_typeSqueueComponentsHeadMap.erase(pair.second);
			}
			return targetComponents;
		}
	}

	Utils::Log::Exception("GameObject " + name + " do not have " + targetType.get_name().to_string() + " Components.");
}

Core::Component::Component* Core::Object::GameObject::GetComponent(rttr::type targetType)
{
	if (!targetType)
	{
		Utils::Log::Exception("Do not have " + targetType.get_name().to_string() + ".");
	}

	if (!targetType.is_derived_from(Core::Component::Component::COMPONENT_TYPE))
	{
		Utils::Log::Exception(targetType.get_name().to_string() + " is not a component.");
	}

	for (const auto& pair : Core::Component::Component::TYPE_MAP)
	{
		if ((targetType == pair.first || targetType.is_base_of(pair.first)) && _typeSqueueComponentsHeadMap.count(pair.second))
		{
			auto node = _typeSqueueComponentsHeadMap[pair.second]->GetIterator().Node();
			Core::Component::Component* found = static_cast<Core::Component::Component*>(node);
			return found;
		}
	}

	Utils::Log::Exception("GameObject " + name + " do not have a " + targetType.get_name().to_string() + " Component.");
}

Core::Component::Component* Core::Object::GameObject::GetComponent(std::string targetTypeName)
{
	return GetComponent(rttr::type::get_by_name(targetTypeName));
}

Core::Component::Component* Core::Object::GameObject::_GetComponent(std::string targetTypeName)
{
	return GetComponent(targetTypeName);
}

std::vector<Core::Component::Component*> Core::Object::GameObject::GetComponents(rttr::type targetType)
{
	if (!targetType)
	{
		Utils::Log::Exception("Do not have " + targetType.get_name().to_string() + ".");
	}

	if (!targetType.is_derived_from(Core::Component::Component::COMPONENT_TYPE))
	{
		Utils::Log::Exception(targetType.get_name().to_string() + " is not a component.");
	}

	auto targetComponents = std::vector<Core::Component::Component*>();

	for (const auto& pair : Core::Component::Component::TYPE_MAP)
	{
		if ((targetType == pair.first || targetType.is_base_of(pair.first)) && _typeSqueueComponentsHeadMap.count(pair.second))
		{
			auto itertor = _typeSqueueComponentsHeadMap[pair.second]->GetIterator();
			while (itertor.IsValid())
			{
				auto foundComponent = static_cast<Component::Component*>(itertor.Node());
				itertor++;

				targetComponents.emplace_back(foundComponent);
			}
		}
	}

	return targetComponents;
}

std::vector<Core::Component::Component*> Core::Object::GameObject::GetComponents(std::string targetTypeName)
{
	return GetComponents(rttr::type::get_by_name(targetTypeName));
}

std::vector<Core::Component::Component*> Core::Object::GameObject::_GetComponents(std::string targetTypeName)
{
	return GetComponents(targetTypeName);
}


bool Core::Object::GameObject::HaveParent()
{
	return _chain.IsParentValid();
}
bool Core::Object::GameObject::HaveChild()
{
	return _chain.IsChildValid();
}
bool Core::Object::GameObject::HaveBrother()
{
	return _chain.IsBrotherValid();
}
Core::Object::GameObject* Core::Object::GameObject::Parent()
{
	return _chain.IsParentValid() ? _chain.Parent()->Object() : nullptr;
}
Core::Object::GameObject* Core::Object::GameObject::Child()
{
	return  _chain.IsChildValid() ? _chain.Child()->Object() : nullptr;
}
Core::Object::GameObject* Core::Object::GameObject::Brother()
{
	return  _chain.IsBrotherValid() ? _chain.Brother()->Object() : nullptr;
}
void Core::Object::GameObject::AddChild(Core::Object::GameObject* child)
{
	this->_chain.AddChild(child->_chain);
}

void Core::Object::GameObject::RemoveChild(Core::Object::GameObject* child)
{
	if (child->Parent() == this)
	{
		child->RemoveSelf();
	}
}

void Core::Object::GameObject::RemoveSelf()
{
	_chain.Remove();
}