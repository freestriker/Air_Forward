#include "core/LoadThread.h"
#include <iostream>
#include <chrono> 
#include <core/SubLoadThread.h>
#include "Graphic/Command/CommandBuffer.h"
#include "core/AssetUtils.h"
#include <glm/glm.hpp>

LoadThread* const LoadThread::instance = new LoadThread();

void LoadThread::Init()
{
}

void LoadThread::OnStart()
{
	_stopped = false;
	_subLoadThreads.emplace_back(new SubLoadThread(*this));
	_subLoadThreads.emplace_back(new SubLoadThread(*this));
	_subLoadThreads.emplace_back(new SubLoadThread(*this));
	_subLoadThreads.emplace_back(new SubLoadThread(*this));
}

void LoadThread::OnRun()
{
	for (auto& subLoadThread : _subLoadThreads)
	{		
		subLoadThread->Start();
	}
	while (!_stopped)
	{
		std::this_thread::sleep_for(std::chrono::seconds(1));
		assetManager->Collect();
	}
}
void LoadThread::OnEnd()
{
	_stopped = true;
	for (auto& subLoadThread : _subLoadThreads)
	{
		subLoadThread->End();
	}
}

LoadThread::LoadThread()
	: Thread()
	, _subLoadThreads()
	, _tasks()
	, _queueMutex()
	, _queueVariable()
	, _stopped(true)
	, assetManager(new AssetManager())
{
}

LoadThread::~LoadThread()
{
	End();

	for (size_t i = 0; i < _subLoadThreads.size(); i++)
	{
		delete _subLoadThreads[i];
	}
	_subLoadThreads.clear();
}

