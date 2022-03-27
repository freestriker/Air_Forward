#pragma once
#include <string>
#include <rttr/type>
#include<vector>
#include <core/Component.h>
class GameObject;
class Component;
class ObjectFactory final
{
public:
	static GameObject* InstantiateGameObject();
	template<typename T>
	static T* InstantiateComponent(std::string typeName, std::vector<rttr::argument> arguments);
	static void DestoryGameObject(GameObject* gameObject);
	static void DestoryComponent(Component* component);
};

template<typename T>
T* ObjectFactory::InstantiateComponent(std::string typeName, std::vector<rttr::argument> arguments)
{
	using namespace rttr;

	static type componentType = type::get<Component>();

	type t = type::get_by_name(typeName);
	if (componentType.is_base_of(t))
	{
		variant var = t.create(arguments);
		T* t = var.get_value< T* >();
		dynamic_cast<Component*>(t)->OnAwake();
		return t;

	}
	else
	{
		assert(false && "It is not a Component.");
	}
}
