#include "Logic/Object/Object.h"
#include <rttr/registration>

RTTR_REGISTRATION
{
	rttr::registration::class_<Logic::Object::Object>("Logic::Object::Object")
		.constructor<>()
		.method("Type", &Logic::Object::Object::Type)
		.method("ToString", &Logic::Object::Object::ToString);
}

Logic::Object::Object::Object()
{
}

Logic::Object::Object::~Object()
{
}

rttr::type Logic::Object::Object::Type()
{
    return rttr::type::get(*this);
}

std::string Logic::Object::Object::ToString()
{
    return Type().get_name().to_string();
}
