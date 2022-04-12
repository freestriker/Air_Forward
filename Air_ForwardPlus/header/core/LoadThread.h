#pragma once
#include "utils/Thread.h"
#include <string>
class LoadThread : public Thread
{
private:
	bool isStop;
	void OnStart()override;
	void OnRun()override;
	void OnEnd() override;
};