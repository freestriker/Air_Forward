#include "core/LoadThread.h"
#include <iostream>
#include <chrono> 
#include <core/SubLoadThread.h>
#include "Graphic/Asset/Texture2D.h"
#include "Graphic/CommandBuffer.h"

LoadThread* const LoadThread::instance = new LoadThread();

void LoadThread::Init()
{
	_subLoadThreads.emplace_back(std::unique_ptr<SubLoadThread>(new SubLoadThread(*this)));
	_subLoadThreads.emplace_back(std::unique_ptr<SubLoadThread>(new SubLoadThread(*this)));
	_subLoadThreads.emplace_back(std::unique_ptr<SubLoadThread>(new SubLoadThread(*this)));
	_subLoadThreads.emplace_back(std::unique_ptr<SubLoadThread>(new SubLoadThread(*this)));
	std::cout << "LoadThread::Init()" << std::endl;
}

void LoadThread::OnStart()
{
	_stopped = false;
	std::cout << "LoadThread::OnStart()" << std::endl;
}

void LoadThread::OnRun()
{
	for (auto& subLoadThread : _subLoadThreads)
	{		
		subLoadThread->Start();
	}
	while (!_stopped)
	{
		std::cout << "LoadThread::OnRun()" << std::endl;
		Graphic::Texture2D texture = Graphic::Texture2D();
		std::string s = std::string("C:\\Users\\FREEstriker\\Desktop\\Screenshot 2022-04-08 201144.png");
		auto result = AddTask([s, &texture](Graphic::CommandBuffer* const cb) {
				Graphic::Texture2D::LoadTexture2D(*cb, s, texture);
			});
		std::this_thread::sleep_for(std::chrono::seconds(1));
		result.get();
	}
}
void LoadThread::OnEnd()
{
	_stopped = true;
	for (auto& subLoadThread : _subLoadThreads)
	{
		subLoadThread->End();
	}
	_subLoadThreads.clear();
	std::cout << "LoadThread::OnEnd()" << std::endl;
}

LoadThread::LoadThread()
	: Thread()
	, _subLoadThreads()
	, _tasks()
	, _queueMutex()
	, _queueVariable()
	, _stopped(true)
{
}

LoadThread::~LoadThread()
{
	End();
}
