#pragma once
#include <rttr/type>

namespace Utils
{
	class ActivableBase
	{
	protected:
		bool _active;
		ActivableBase();
		ActivableBase(bool active);
		~ActivableBase();
	public:
		bool Active();
		void SetActive(bool active);

		RTTR_ENABLE()
	};
}