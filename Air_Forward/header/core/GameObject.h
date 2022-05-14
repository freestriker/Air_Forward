#pragma once
#include "core/object.h"
#include "utils/ChildBrotherTree.h"
#include <vector>
#include <Component/Transform/Transform.h>
#include <rttr/type>
#include "core/LifeTime.h"
class Component;
class GameObject final: public Object, private LifeTime
{
	friend class ObjectFactory;

private:
	std::vector<Component*> components;
	ChildBrotherTree<GameObject> chain;
	bool active;

	GameObject(std::string name);
	GameObject();
	virtual ~GameObject();

	void OnAwake()override;
	void OnEnable()override;
	void OnDisable()override;
	void OnDestory()override;

public:
	std::string name;
	Transform transform;

	void SetActive(bool active);
	bool Active();

	Component* GetComponent(std::string typeName);
	std::vector<Component*> GetComponents(std::string typeName);
	void AddComponent(Component* component);
	void RemoveComponent(Component* component);
	Component* RemoveComponent(std::string typeName);
	std::vector<Component*> RemoveComponents(std::string typeName);

	GameObject* Parent();
	GameObject* Child();
	GameObject* Brother();
	void AddChild(GameObject* child);
	void RemoveChild(GameObject* child);
	void RemoveSelf();

	RTTR_ENABLE(Object)
};