#pragma once
#include<thread>
class Thread
{
private:
	std::thread _thread;

public:
	Thread();
	virtual ~Thread();
	void Start();
	void End();
protected:
	virtual void OnStart();
	virtual void OnRun();
	virtual void OnEnd();
private:
	void Run();
	Thread(const Thread&) = delete;
	Thread& operator=(const Thread&) = delete;
	Thread(Thread&&) = delete;
	Thread& operator=(Thread&&) = delete;
};