#include "core/LoadThread.h"
#include <iostream>
#include <chrono> 
#include <core/SubLoadThread.h>
#include "Graphic/Asset/Texture2D.h"

LoadThread* const LoadThread::instance = new LoadThread();

void LoadThread::OnStart()
{
	stop = false;
	std::cout << "LoadThread::OnStart()" << std::endl;
}

void LoadThread::OnRun()
{
	for (auto& subLoadThread : subLoadThreads)
	{
		subLoadThread.PopulateLoadThread(this);
		subLoadThread.Start();
	}
	while (!stop)
	{
		std::cout << "LoadThread::OnRun()" << std::endl;
		Texture2D texture = Texture2D();
		std::string s = std::string("C:\\Users\\FREEstriker\\Desktop\\Screenshot 2022-04-08 201144.png");
		auto result = AddTask([s, &texture](VkCommandBuffer cb) {
				Texture2D::LoadTexture2D(cb, s, texture);
			});
		std::this_thread::sleep_for(std::chrono::seconds(1));

	}
}
void LoadThread::OnEnd()
{
	stop = true;
	for (auto& subLoadThread : subLoadThreads)
	{
		subLoadThread.End();
	}
	std::cout << "LoadThread::OnEnd()" << std::endl;
}

LoadThread::LoadThread(): Thread(), subLoadThreads(4), stop(true)
{
}

LoadThread::~LoadThread()
{
	End();
}
