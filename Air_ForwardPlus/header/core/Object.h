#pragma once
#include <typeinfo>
#include <string>
#include <rttr/type>

class Object
{
public:
	Object();
	virtual ~Object();
	virtual const type_info& Type()final;
	virtual std::string ToString();

	RTTR_ENABLE()
};