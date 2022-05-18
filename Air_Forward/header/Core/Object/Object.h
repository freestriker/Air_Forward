#pragma once
#include <typeinfo>
#include <string>
#include <rttr/type>

namespace Core
{
	namespace Object
	{
		class Object
		{
		public:
			Object();
			virtual ~Object();
			rttr::type Type();
			virtual std::string ToString();

			RTTR_ENABLE()
		};
	}
}
