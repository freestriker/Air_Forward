#pragma once
#include "core/Object.h"
#include "core/LifeTime.h"

class GameObject;
class Component : public Object, protected LifeTime
{
	friend class GameObject;
	friend class ObjectFactory;
private:
	bool active;
	GameObject* gameObject;

public:
	Component();
	virtual ~Component();
	bool Active();
	void SetActive(bool active);
	GameObject* GameObject();
	RTTR_ENABLE(Object)
};