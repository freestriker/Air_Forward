#pragma once
#include <typeinfo>
#include <string>
class Object
{
public:
	Object();
	~Object();
	virtual const type_info& Type()final;
	virtual std::string ToString();
};