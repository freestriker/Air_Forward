#include "Core/Object/LifeCycle.h"

void Core::Object::LifeCycle::Awake()
{
	_neverStarted = true;
	if (_neverAwaked)
	{
		_neverAwaked = false;
		OnAwake();
	}
}

void Core::Object::LifeCycle::Update()
{
	if (_neverStarted)
	{
		_neverStarted = false;
		OnStart();
	}
	OnUpdate();
}

Core::Object::LifeCycle::LifeCycle()
	: _neverStarted(true)
	, _neverAwaked(true)
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

void Core::Object::LifeCycle::OnDestroy()
{
}
