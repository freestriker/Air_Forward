#pragma once
#include "utils/ChildBrotherTree.h"
class GameObject;
class Global
{
public:
	GameObject* rootGameObject;
	Global();
	~Global();
};
extern Global global;
