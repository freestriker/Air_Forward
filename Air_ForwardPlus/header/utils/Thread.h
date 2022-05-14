#pragma once
#include<thread>
class Thread
{
private:
	std::thread _thread;

public:
	Thread();
	virtual ~Thread();
	virtual void Init();
	void Start();
	void End();
	void WaitForStartFinish();
protected:
	virtual void OnStart();
	virtual void OnThreadStart();
	virtual void OnRun();
	virtual void OnEnd();
private:
	bool _finishOnThreadStart;
	void Run();
	Thread(const Thread&) = delete;
	Thread& operator=(const Thread&) = delete;
	Thread(Thread&&) = delete;
	Thread& operator=(Thread&&) = delete;
};
using _Thread = Thread;