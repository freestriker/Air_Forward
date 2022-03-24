#include "core/object.h"
#include <rttr/registration>

RTTR_REGISTRATION
{
	rttr::registration::class_<Object>("Object")
		.method("Type", &Object::Type)
		.method("ToString", &Object::ToString);
}

Object::Object()
{
}

Object::~Object()
{
}

const type_info& Object::Type()
{
    return typeid(*this);
}

std::string Object::ToString()
{
    return Type().name();
}
