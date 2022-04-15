#include "core/LoadThread.h"
#include <iostream>
#include <chrono> 
#include <core/SubLoadThread.h>
#include "Graphic/Asset/Texture2D.h"
#include "Graphic/CommandBuffer.h"

LoadThread* const LoadThread::instance = new LoadThread();

void LoadThread::OnStart()
{
	stop = false;
	std::cout << "LoadThread::OnStart()" << std::endl;
}

void LoadThread::OnRun()
{
	subLoadThreads.emplace_back(*this);
	subLoadThreads.emplace_back(*this);
	subLoadThreads.emplace_back(*this);
	subLoadThreads.emplace_back(*this);
	for (auto& subLoadThread : subLoadThreads)
	{		
		subLoadThread.Start();
	}
	while (!stop)
	{
		std::cout << "LoadThread::OnRun()" << std::endl;
		Graphic::Texture2D texture = Graphic::Texture2D();
		std::string s = std::string("C:\\Users\\FREEstriker\\Desktop\\Screenshot 2022-04-08 201144.png");
		auto result = AddTask([s, &texture](Graphic::CommandBuffer& cb) {
				Graphic::Texture2D::LoadTexture2D(cb, s, texture);
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
