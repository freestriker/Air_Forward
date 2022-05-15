#include "Core/Object/LifeCycle.h"
#include <rttr/registration>

RTTR_REGISTRATION
{
	rttr::registration::class_<Core::Object::LifeCycle>("Core::Object::LifeCycle")
		.method("Active", &Core::Object::LifeCycle::Active)
		.method("SetActive", &Core::Object::LifeCycle::SetActive);
}

Core::Object::LifeCycle::LifeCycle()
	:_active(false)
{
}

Core::Object::LifeCycle::~LifeCycle()
{
}

void Core::Object::LifeCycle::OnAwake()
{
}

void Core::Object::LifeCycle::OnStart()
{
}

void Core::Object::LifeCycle::OnUpdate()
{
}

void Core::Object::LifeCycle::OnDestory()
{
}

void Core::Object::LifeCycle::OnEnable()
{
}

void Core::Object::LifeCycle::OnDisable()
{
}

bool Core::Object::LifeCycle::OnCheckValid()
{
	return true;
}

bool Core::Object::LifeCycle::Active()
{
	return _active;
}

void Core::Object::LifeCycle::SetActive(bool active)
{
	if (_active != active)
	{
		_active = active;
		if (OnCheckValid())
		{
			if (_active)
			{
				OnEnable();
			}
			else
			{
				OnDisable();
			}
		}
	}
}
