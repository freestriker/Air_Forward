#include "core/object.h"

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
