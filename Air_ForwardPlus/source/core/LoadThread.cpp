#include "core/LoadThread.h"
#include <iostream>
#include <chrono> 
void LoadThread::OnStart()
{
	std::cout << "LoadThread::OnStart()" << std::endl;
	isStop = false;
}

void LoadThread::OnRun()
{
	while (!isStop)
	{
		std::cout << "LoadThread::OnRun()" << std::endl;
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}
}

void LoadThread::OnEnd()
{
	isStop = true;
	std::cout << "LoadThread::OnEnd()" << std::endl;
}
