#pragma once
#include "core/Object.h"

class GameObject;
class Component : public Object
{
public:
	GameObject* gameObject;
	Component();
	virtual ~Component();
	virtual void UpdateSelf(GameObject* parentGameObject);
	virtual void OnAdd();
	RTTR_ENABLE()
};