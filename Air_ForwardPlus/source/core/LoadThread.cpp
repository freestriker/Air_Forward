#include "core/LoadThread.h"
#include <iostream>
#include <chrono> 
#include <core/SubLoadThread.h>
#include "Graphic/Asset/Texture2D.h"
#include "Graphic/CommandBuffer.h"

LoadThread* const LoadThread::instance = new LoadThread();

void LoadThread::Init()
{
	_subLoadThreads.emplace_back(new SubLoadThread(*this));
	_subLoadThreads.emplace_back(new SubLoadThread(*this));
	_subLoadThreads.emplace_back(new SubLoadThread(*this));
	_subLoadThreads.emplace_back(new SubLoadThread(*this));
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
		std::this_thread::sleep_for(std::chrono::seconds(1));
		auto r1 = Graphic::Texture2D::LoadAsync("C:\\Users\\FREEstriker\\Desktop\\Screenshot 2022-04-08 201144.png");
		auto r2 = Graphic::Texture2D::LoadAsync("C:\\Users\\FREEstriker\\Desktop\\Screenshot 2022-04-08 201144.png");
		auto r3 = Graphic::Texture2D::LoadAsync("C:\\Users\\FREEstriker\\Desktop\\Screenshot 2022-04-08 201144.png");
		auto r4 = Graphic::Texture2D::LoadAsync("C:\\Users\\FREEstriker\\Desktop\\Screenshot 2022-04-08 201144.png");
	}
}
void LoadThread::OnEnd()
{
	_stopped = true;
	for (auto& subLoadThread : _subLoadThreads)
	{
		subLoadThread->End();
	}
	std::cout << "LoadThread::OnEnd()" << std::endl;
}

LoadThread::LoadThread()
	: Thread()
	, _subLoadThreads()
	, _tasks()
	, _queueMutex()
	, _queueVariable()
	, _stopped(true)
	, assetInstanceManager()
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

AssetInstanceManager::AssetInstanceManager()
	: _warps()
	, mutex()
{
}

AssetInstanceManager::~AssetInstanceManager()
{
	for (const auto& pair : _warps)
	{
		delete pair.second.assetInstance;
	}
}

void AssetInstanceManager::AddInstance(std::string path, void* assetInstance)
{
	_warps.emplace(path, AssetInstanceWarp{ path , 0, assetInstance });
}

void* AssetInstanceManager::GetInstance(std::string path)
{
	_warps[path].refCount++;
	return _warps[path].assetInstance;
}

bool AssetInstanceManager::ContainsInstance(std::string path)
{
	return _warps.count(path);
}

void AssetInstanceManager::RecycleInstance(std::string path)
{
	--_warps[path].refCount;
}
