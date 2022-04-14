#pragma once
#include "utils/Thread.h"
#include <string>
#include "utils/ThreadPool.h"
class LoadThread : public Thread, public ThreadPool
{
private:
	bool isStop;
	void OnStart()override;
	void OnRun()override;
	void OnEnd() override;
	LoadThread();
	~LoadThread();
public:
	static LoadThread* const instance;
};