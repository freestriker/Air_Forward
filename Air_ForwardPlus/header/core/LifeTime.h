#pragma once
#include <rttr/type>

class LifeTime
{
public:
	LifeTime();
	~LifeTime();
	virtual void OnAwake();
	virtual void OnStart();
	virtual void OnUpdate();
	virtual void OnDestory();
	virtual void OnEnable();
	virtual void OnDisable();

	RTTR_ENABLE()

};