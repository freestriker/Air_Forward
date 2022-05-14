#include "utils/ThreadBase.h"
#include <iostream>

void ThreadBase::OnStart()
{
}

void ThreadBase::OnThreadStart()
{
}

//void Thread::OnRun()
//{
//}

void ThreadBase::OnEnd()
{
}

ThreadBase::ThreadBase()
	: _thread()
	, _finishOnThreadStart(false)
{

}

ThreadBase::~ThreadBase()
{

}

void ThreadBase::Init()
{
}

void ThreadBase::Start()
{
	_finishOnThreadStart = false;
	OnStart();
	std::thread t(&ThreadBase::Run, this);
	_thread.swap(t);
}

void ThreadBase::End()
{
	OnEnd();
	if (_thread.joinable())
	{
		_thread.join();
	}
}

void ThreadBase::WaitForStartFinish()
{
	while (!_finishOnThreadStart)
	{
		std::this_thread::yield();
	}
}

void ThreadBase::Run()
{
	OnThreadStart();
	_finishOnThreadStart = true;
	OnRun();
}
