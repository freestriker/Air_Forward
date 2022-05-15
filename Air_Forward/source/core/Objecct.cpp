#include "core/object.h"
#include <rttr/registration>

RTTR_REGISTRATION
{
	rttr::registration::class_<Object>("Object")
		.constructor<>()
		.method("TypeName", &Object::TypeName)
		.method("ToString", &Object::ToString);
}

Object::Object()
{
}

Object::~Object()
{
}

std::string Object::TypeName()
{
    return typeid(*this).name();
}

std::string Object::ToString()
{
    return TypeName();
}
