#include "Core/Object/Object.h"
#include <rttr/registration>

RTTR_REGISTRATION
{
	rttr::registration::class_<Core::Object::Object>("Core::Object::Object")
		.constructor<>()
		.method("TypeName", &Core::Object::Object::TypeName)
		.method("ToString", &Core::Object::Object::ToString);
}

Core::Object::Object::Object()
{
}

Core::Object::Object::~Object()
{
}

std::string Core::Object::Object::TypeName()
{
    return typeid(*this).name();
}

std::string Core::Object::Object::ToString()
{
    return TypeName();
}
