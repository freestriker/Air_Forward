#include <core/Global.h>
#include "core/GameObject.h"
#include "core/ObjectFactory.h"
Global global = Global();

Global::Global()
{
	rootGameObject = ObjectFactory::InstantiateGameObject();
}

Global::~Global()
{
	ObjectFactory::DestoryGameObject(rootGameObject);
}
