#include <core/Global.h>
#include "core/GameObject.h"
Global global = Global();

Global::Global()
{
	rootGameObject = new GameObject();
	rootGameObject->chain = new ChildBrotherTree<GameObject>(rootGameObject);
}

Global::~Global()
{
	delete rootGameObject;
}
