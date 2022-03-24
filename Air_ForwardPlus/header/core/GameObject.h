#pragma once
#include "core/object.h"
#include "utils/ChildBrotherTree.h"
#include <vector>
#include <Component/Transform/Transform.h>
#include <rttr/type>
class Component;
class GameObject: public Object
{
	std::vector<Component*> components;
public:
	std::string name;
	ChildBrotherTree<GameObject>* chain;
	Transform transform;

	GameObject(std::string name);
	GameObject();
	virtual ~GameObject();

	void UpdateSelf(GameObject* parentGameObject);
	void UpdateSelfWithoutTransform(GameObject* parentGameObject);
	void CascadeUpdate(GameObject* parentGameObject);


	Component* GetComponent(std::string typeName);
	std::vector<Component*> GetComponents(std::string typeName);
	void AddComponent(Component* component);
	void RemoveComponent(Component* component);
	Component* RemoveComponent(std::string typeName);
	std::vector<Component*> RemoveComponents(std::string typeName);

	void AddChild(GameObject* child);
	void RemoveChild(GameObject* child);

	RTTR_ENABLE(Object)
};