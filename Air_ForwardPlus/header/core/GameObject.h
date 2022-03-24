#pragma once
#include "core/object.h"
#include "utils/ChildBrotherTree.h"
#include <vector>
class Component;
class GameObject : public ChildBrotherTree<GameObject>, public Object
{
	std::vector<Component*> components;
public:
	std::string name;

	GameObject();
	virtual ~GameObject();
	GameObject(std::string name);
	void UpdateSelf(GameObject* parentGameObject);
	void UpdateSelfWithoutTransform(GameObject* parentGameObject);
	void CascadeUpdate(GameObject* parentGameObject);
	void OnAddedAsChild(GameObject* parentGameObject);
	template<typename T>
	T* GetComponent(std::string typeName);
	template<typename T>
	std::vector<T*> GetComponents(std::string typeName);
	void AddComponent(Component* component);
	void RemoveComponent(Component* component);
	template<typename T>
	T* RemoveComponent(std::string typeName);
	template<typename T>
	std::vector<T*> RemoveComponents(std::string typeName);

	RTTR_ENABLE(Object)
};

template<typename T>
inline T* GameObject::GetComponent(std::string typeName)
{
	return nullptr;
}

template<typename T>
inline std::vector<T*> GameObject::GetComponents(std::string typeName)
{
	return std::vector<T*>();
}

template<typename T>
inline T* GameObject::RemoveComponent(std::string typeName)
{
	return nullptr;
}

template<typename T>
inline std::vector<T*> GameObject::RemoveComponents(std::string typeName)
{
	return std::vector<T*>();
}
