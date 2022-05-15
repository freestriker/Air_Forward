#include "Utils/ThreadBase.h"
#include <iostream>

void Utils::ThreadBase::OnStart()
{
}

void Utils::ThreadBase::OnThreadStart()
{
}

void Utils::ThreadBase::OnEnd()
{
}

Utils::ThreadBase::ThreadBase()
	: _thread()
	, _finishOnThreadStart(false)
{

}

Utils::ThreadBase::~ThreadBase()
{

}

void Utils::ThreadBase::Init()
{
}

void Utils::ThreadBase::Start()
{
	_finishOnThreadStart = false;
	OnStart();
	std::thread t(&ThreadBase::Run, this);
	_thread.swap(t);
}

void Utils::ThreadBase::End()
{
	OnEnd();
	if (_thread.joinable())
	{
		_thread.join();
	}
}

void Utils::ThreadBase::WaitForStartFinish()
{
	while (!_finishOnThreadStart)
	{
		std::this_thread::yield();
	}
}

void Utils::ThreadBase::Run()
{
	OnThreadStart();
	_finishOnThreadStart = true;
	OnRun();
}
