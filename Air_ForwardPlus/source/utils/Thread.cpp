#include "utils/Thread.h"

void Thread::OnStart()
{
}

void Thread::OnRun()
{
}

void Thread::OnEnd()
{
}

Thread::Thread(): _thread()
{

}

Thread::~Thread()
{
	End();
}

void Thread::Start()
{
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

void Thread::Run()
{
	OnRun();
}
