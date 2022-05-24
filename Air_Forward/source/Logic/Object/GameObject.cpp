#include "Logic/Object/GameObject.h"
#include "Logic/Component/Component.h"
#include <cassert>
#include <rttr/registration>
#include "Utils/Log.h"
#include "Logic/Core/Instance.h"

RTTR_REGISTRATION
{
	using namespace rttr;
	registration::class_<Logic::Object::GameObject>("Logic::Object::GameObject")
		.method("AddComponent", &Logic::Object::GameObject::AddComponent)
		.method("RemoveComponent", &Logic::Object::GameObject::_RemoveComponent1)
		.method("RemoveComponentByTypeName", &Logic::Object::GameObject::_RemoveComponent2)
		.method("RemoveComponents", &Logic::Object::GameObject::_RemoveComponents1)
		.method("RemoveComponentsByTypeName", &Logic::Object::GameObject::_RemoveComponents2)
		.method("GetComponent", &Logic::Object::GameObject::_GetComponent)
		.method("GetComponents", &Logic::Object::GameObject::_GetComponents)
		.method("Parent", &Logic::Object::GameObject::Parent)
		.method("Child", &Logic::Object::GameObject::Child)
		.method("Brother", &Logic::Object::GameObject::Brother)
		.method("AddChild", &Logic::Object::GameObject::AddChild)
		.method("RemoveChild", &Logic::Object::GameObject::RemoveChild)
		.method("RemoveSelf", &Logic::Object::GameObject::RemoveSelf)
		;
}

Logic::Object::GameObject::GameObject(std::string name)
	: Utils::ActivableBase()
	, Object()
	, name(name)
	, _timeSqueueComponentsHead()
	, _typeSqueueComponentsHeadMap()
	, transform()
	, _chain()
{
	_chain.SetObject(this);
	transform._gameObject = this;
}

Logic::Object::GameObject::GameObject()
	: GameObject("New GameObject")
{
}

Logic::Object::GameObject::~GameObject()
{

}

void Logic::Object::GameObject::AddComponent(Logic::Component::Component* targetComponent)
{
	_timeSqueueComponentsHead.Add(targetComponent);
	if (!_typeSqueueComponentsHeadMap.count(targetComponent->_type))
	{
		_typeSqueueComponentsHeadMap[targetComponent->_type] = std::unique_ptr<Utils::CrossLinkableRowHead>(new Utils::CrossLinkableRowHead());
	}
	_typeSqueueComponentsHeadMap[targetComponent->_type]->Add(targetComponent);
	targetComponent->_gameObject = this;
}

void Logic::Object::GameObject::RemoveComponent(Logic::Component::Component* targetComponent)
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

	if (Logic::Core::Instance::_validComponentInIteration.count(targetComponent))
	{
		Logic::Core::Instance::_validComponentInIteration.erase(targetComponent);
	}
}

Logic::Component::Component* Logic::Object::GameObject::RemoveComponent(std::string targetTypeName)
{
	return RemoveComponent(rttr::type::get_by_name(targetTypeName));
}

void Logic::Object::GameObject::_RemoveComponent1(Logic::Component::Component* component)
{
	RemoveComponent(component);
}

Logic::Component::Component* Logic::Object::GameObject::_RemoveComponent2(std::string typeName)
{
	return RemoveComponent(typeName);
}

Logic::Component::Component* Logic::Object::GameObject::RemoveComponent(rttr::type targetType)
{
	if (!targetType)
	{
		Utils::Log::Exception("Do not have " + targetType.get_name().to_string() + ".");
	}

	if (!targetType.is_derived_from(Logic::Component::Component::COMPONENT_TYPE))
	{
		Utils::Log::Exception(targetType.get_name().to_string() + " is not a component.");
	}

	for (const auto& pair : Logic::Component::Component::TYPE_MAP)
	{
		if ((targetType == pair.first || targetType.is_base_of(pair.first)) && _typeSqueueComponentsHeadMap.count(pair.second))
		{
			Logic::Component::Component* found = static_cast<Logic::Component::Component*>(_typeSqueueComponentsHeadMap[pair.second]->GetIterator().Node());
			RemoveComponent(found);
			return found;
		}
		else if (pair.first.is_base_of(targetType) && _typeSqueueComponentsHeadMap.count(pair.second))
		{
			for (auto iterator = _typeSqueueComponentsHeadMap[pair.second]->GetIterator(); iterator.IsValid(); iterator++)
			{
				Logic::Component::Component* found = static_cast<Logic::Component::Component*>(iterator.Node());
				if (targetType.is_base_of(found->Type()))
				{
					RemoveComponent(found);
					return found;
				}
			}
		}
	}

	Utils::Log::Exception("GameObject " + name + " do not have a " + targetType.get_name().to_string() + " Component.");
}

void Logic::Object::GameObject::RemoveComponents(std::vector<Component::Component*> components)
{
	for (const auto& component : components)
	{
		RemoveComponent(component);
	}
}

std::vector<Logic::Component::Component*> Logic::Object::GameObject::RemoveComponents(std::string targetTypeName)
{
	return RemoveComponents(rttr::type::get_by_name(targetTypeName));
}

void Logic::Object::GameObject::_RemoveComponents1(std::vector<Component::Component*> components)
{
	RemoveComponents(components);
}

std::vector<Logic::Component::Component*> Logic::Object::GameObject::_RemoveComponents2(std::string typeName)
{
	return RemoveComponents(typeName);
}

std::vector<Logic::Component::Component*> Logic::Object::GameObject::RemoveComponents(rttr::type targetType)
{
	if (!targetType)
	{
		Utils::Log::Exception("Do not have " + targetType.get_name().to_string() + ".");
	}

	if (!targetType.is_derived_from(Logic::Component::Component::COMPONENT_TYPE))
	{
		Utils::Log::Exception(targetType.get_name().to_string() + " is not a component.");
	}

	auto targetComponents = std::vector<Logic::Component::Component*>();
	for (const auto& pair : Logic::Component::Component::TYPE_MAP)
	{
		if ((targetType == pair.first || targetType.is_base_of(pair.first)) && _typeSqueueComponentsHeadMap.count(pair.second))
		{
			for (auto itertor = _typeSqueueComponentsHeadMap[pair.second]->GetIterator(); itertor.IsValid(); )
			{
				auto foundComponent = static_cast<Component::Component*>(itertor.Node());

				_timeSqueueComponentsHead.Remove(foundComponent);
				itertor = _typeSqueueComponentsHeadMap[pair.second]->Remove(itertor);
				foundComponent->_gameObject = nullptr;

				targetComponents.emplace_back(foundComponent);
			}
			if (!_typeSqueueComponentsHeadMap[pair.second]->HaveNode())
			{
				_typeSqueueComponentsHeadMap.erase(pair.second);
			}
		}
		else if (pair.first.is_base_of(targetType) && _typeSqueueComponentsHeadMap.count(pair.second))
		{
			for (auto iterator = _typeSqueueComponentsHeadMap[pair.second]->GetIterator(); iterator.IsValid(); )
			{
				Logic::Component::Component* found = static_cast<Logic::Component::Component*>(iterator.Node());
				if (targetType.is_base_of(found->Type()))
				{
					_timeSqueueComponentsHead.Remove(found);
					iterator = _typeSqueueComponentsHeadMap[pair.second]->Remove(iterator);
					found->_gameObject = nullptr;

					targetComponents.emplace_back(found);
				}
			}
			if (!_typeSqueueComponentsHeadMap[pair.second]->HaveNode())
			{
				_typeSqueueComponentsHeadMap.erase(pair.second);
			}
		}
	}

	return targetComponents;
}

Logic::Component::Component* Logic::Object::GameObject::GetComponent(rttr::type targetType)
{
	if (!targetType)
	{
		Utils::Log::Exception("Do not have " + targetType.get_name().to_string() + ".");
	}

	if (!Logic::Component::Component::COMPONENT_TYPE.is_base_of(targetType))
	{
		Utils::Log::Exception(targetType.get_name().to_string() + " is not a component.");
	}

	for (const auto& pair : Logic::Component::Component::TYPE_MAP)
	{
		if ((targetType == pair.first || targetType.is_base_of(pair.first)) && _typeSqueueComponentsHeadMap.count(pair.second))
		{
			auto node = _typeSqueueComponentsHeadMap[pair.second]->GetIterator().Node();
			Logic::Component::Component* found = static_cast<Logic::Component::Component*>(node);
			return found;
		}
		else if (pair.first.is_base_of(targetType) && _typeSqueueComponentsHeadMap.count(pair.second))
		{
			for (auto iterator = _typeSqueueComponentsHeadMap[pair.second]->GetIterator(); iterator.IsValid(); iterator++)
			{
				Logic::Component::Component* found = static_cast<Logic::Component::Component*>(iterator.Node());
				if(targetType.is_base_of(found->Type())) return found;
			}
		}
	}

	Utils::Log::Exception("GameObject " + name + " do not have a " + targetType.get_name().to_string() + " Component.");
}

Logic::Component::Component* Logic::Object::GameObject::GetComponent(std::string targetTypeName)
{
	return GetComponent(rttr::type::get_by_name(targetTypeName));
}

Logic::Component::Component* Logic::Object::GameObject::_GetComponent(std::string targetTypeName)
{
	return GetComponent(targetTypeName);
}

std::vector<Logic::Component::Component*> Logic::Object::GameObject::GetComponents(rttr::type targetType)
{
	if (!targetType)
	{
		Utils::Log::Exception("Do not have " + targetType.get_name().to_string() + ".");
	}

	if (!targetType.is_derived_from(Logic::Component::Component::COMPONENT_TYPE))
	{
		Utils::Log::Exception(targetType.get_name().to_string() + " is not a component.");
	}

	auto targetComponents = std::vector<Logic::Component::Component*>();

	for (const auto& pair : Logic::Component::Component::TYPE_MAP)
	{
		if ((targetType == pair.first || targetType.is_base_of(pair.first)) && _typeSqueueComponentsHeadMap.count(pair.second))
		{
			for (auto itertor = _typeSqueueComponentsHeadMap[pair.second]->GetIterator(); itertor.IsValid(); )
			{
				auto foundComponent = static_cast<Component::Component*>(itertor.Node());

				targetComponents.emplace_back(foundComponent);
			}
		}
		else if (pair.first.is_base_of(targetType) && _typeSqueueComponentsHeadMap.count(pair.second))
		{
			for (auto iterator = _typeSqueueComponentsHeadMap[pair.second]->GetIterator(); iterator.IsValid(); iterator++)
			{
				Logic::Component::Component* found = static_cast<Logic::Component::Component*>(iterator.Node());
				if (targetType.is_base_of(found->Type()))
				{
					targetComponents.emplace_back(found);
				}
			}
		}
	}

	return targetComponents;
}

std::vector<Logic::Component::Component*> Logic::Object::GameObject::GetComponents(std::string targetTypeName)
{
	return GetComponents(rttr::type::get_by_name(targetTypeName));
}

std::vector<Logic::Component::Component*> Logic::Object::GameObject::_GetComponents(std::string targetTypeName)
{
	return GetComponents(targetTypeName);
}

bool Logic::Object::GameObject::HaveParent()
{
	return _chain.IsParentValid();
}

bool Logic::Object::GameObject::HaveChild()
{
	return _chain.IsChildValid();
}

bool Logic::Object::GameObject::HaveBrother()
{
	return _chain.IsBrotherValid();
}

Logic::Object::GameObject* Logic::Object::GameObject::Parent()
{
	return _chain.IsParentValid() ? _chain.Parent()->Object() : nullptr;
}

Logic::Object::GameObject* Logic::Object::GameObject::Child()
{
	return  _chain.IsChildValid() ? _chain.Child()->Object() : nullptr;
}

Logic::Object::GameObject* Logic::Object::GameObject::Brother()
{
	return  _chain.IsBrotherValid() ? _chain.Brother()->Object() : nullptr;
}

void Logic::Object::GameObject::AddChild(Logic::Object::GameObject* child)
{
	this->_chain.AddChild(child->_chain);
}

void Logic::Object::GameObject::RemoveChild(Logic::Object::GameObject* child)
{
	if (child->Parent() == this)
	{
		child->RemoveSelf();
	}
}

void Logic::Object::GameObject::RemoveSelf()
{
	_chain.Remove();
	if (Logic::Core::Instance::_validGameObjectInIteration.count(this))
	{
		Logic::Core::Instance::_validGameObjectInIteration.erase(this);
	}
}