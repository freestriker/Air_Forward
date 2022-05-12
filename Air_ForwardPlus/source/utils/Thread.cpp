#include "utils/Thread.h"
#include <iostream>

void Thread::OnStart()
{
}

void Thread::OnThreadStart()
{
}

void Thread::OnRun()
{
}

void Thread::OnEnd()
{
}

Thread::Thread()
	: _thread()
	, _finishOnThreadStart(false)
{

}

Thread::~Thread()
{

}

void Thread::Init()
{
}

void Thread::Start()
{
	_finishOnThreadStart = false;
	OnStart();
	std::thread t(&Thread::Run, this);
	_thread.swap(t);
}

void Thread::End()
{
	OnEnd();
	if (_thread.joinable())
	{
		_thread.join();
	}
}

void Thread::WaitForStartFinish()
{
	while (!_finishOnThreadStart)
	{
		std::this_thread::yield();
	}
}

void Thread::Run()
{
	OnThreadStart();
	_finishOnThreadStart = true;
	OnRun();
}
