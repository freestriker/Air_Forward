#include "Core/Object/Object.h"
#include <rttr/registration>

RTTR_REGISTRATION
{
	rttr::registration::class_<Core::Object::Object>("Core::Object::Object")
		.constructor<>()
		.method("Type", &Core::Object::Object::Type)
		.method("ToString", &Core::Object::Object::ToString);
}

Core::Object::Object::Object()
{
}

Core::Object::Object::~Object()
{
}

rttr::type Core::Object::Object::Type()
{
    return rttr::type::get(*this);
}

std::string Core::Object::Object::ToString()
{
    return Type().get_name().to_string();
}
