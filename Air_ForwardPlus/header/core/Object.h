#pragma once
#include <typeinfo>
#include <string>
#include <rttr/type>

class Object
{
public:
	Object();
	virtual ~Object();
	std::string TypeName();
	virtual std::string ToString();

	RTTR_ENABLE()
};