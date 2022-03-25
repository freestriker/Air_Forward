#pragma once
#include <string>
#include <rttr/type>
#include<vector>
class GameObject;
class Component;
class ObjectFactory
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
	variant var = type::get_by_name(typeName).create(arguments);
	Component* c = rttr::rttr_cast<Component*>(var.get_raw_ptr());
	c->OnAwake();
	return rttr::rttr_cast<Component*>(c);
}
