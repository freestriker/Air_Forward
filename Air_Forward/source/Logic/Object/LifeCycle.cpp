#include "Logic/Object/LifeCycle.h"

void Logic::Object::LifeCycle::Awake()
{
	_neverStarted = true;
	if (_neverAwaked)
	{
		_neverAwaked = false;
		OnAwake();
	}
}

void Logic::Object::LifeCycle::Update()
{
	if (_neverStarted)
	{
		_neverStarted = false;
		OnStart();
	}
	OnUpdate();
}

Logic::Object::LifeCycle::LifeCycle()
	: _neverStarted(true)
	, _neverAwaked(true)
{
}

Logic::Object::LifeCycle::~LifeCycle()
{
}

void Logic::Object::LifeCycle::OnAwake()
{
}

void Logic::Object::LifeCycle::OnStart()
{
}

void Logic::Object::LifeCycle::OnUpdate()
{
}

void Logic::Object::LifeCycle::OnDestroy()
{
}
