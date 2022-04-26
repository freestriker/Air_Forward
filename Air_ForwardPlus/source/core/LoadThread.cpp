#include "core/LoadThread.h"
#include <iostream>
#include <chrono> 
#include <core/SubLoadThread.h>
#include "Graphic/Asset/Texture2D.h"
#include "Graphic/CommandBuffer.h"
#include "core/AssetUtils.h"
#include "Graphic/Asset/Mesh.h"
#include "Graphic/Asset/Shader.h"

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
		{
			auto r1 = Graphic::Texture2D::LoadAsync("C:\\Users\\FREEstriker\\Desktop\\Screenshot 2022-04-08 201144.png");
			auto r2 = Graphic::Texture2D::LoadAsync("C:\\Users\\FREEstriker\\Desktop\\Screenshot 2022-04-08 201144.png");
			auto r3 = Graphic::Texture2D::LoadAsync("C:\\Users\\FREEstriker\\Desktop\\Screenshot 2022-04-08 201144.png");
			auto r4 = Graphic::Texture2D::LoadAsync("C:\\Users\\FREEstriker\\Desktop\\Screenshot 2022-04-08 201144.png");

			auto r5 = Graphic::Texture2D::Load("C:\\Users\\FREEstriker\\Desktop\\Screenshot 2022-04-08 201144.png");
			auto r6 = Graphic::Texture2D::Load("C:\\Users\\FREEstriker\\Desktop\\Screenshot 2022-04-08 201144.png");

			auto r7 = new Graphic::Texture2D(*r5);
			auto r8 = new Graphic::Texture2D(*r5);

			delete r5;
			delete r8;
		}

		{
			auto r1 = Graphic::Mesh::LoadAsync("C:\\Users\\FREEstriker\\Desktop\\Flat_Wall_Normal.ply");
			auto r2 = Graphic::Mesh::LoadAsync("C:\\Users\\FREEstriker\\Desktop\\Flat_Wall_Normal.ply");
			auto r3 = Graphic::Mesh::LoadAsync("C:\\Users\\FREEstriker\\Desktop\\Flat_Wall_Normal.ply");
			auto r4 = Graphic::Mesh::LoadAsync("C:\\Users\\FREEstriker\\Desktop\\Flat_Wall_Normal.ply");

			auto r5 = Graphic::Mesh::Load("C:\\Users\\FREEstriker\\Desktop\\Flat_Wall_Normal.ply");
			auto r6 = Graphic::Mesh::Load("C:\\Users\\FREEstriker\\Desktop\\Flat_Wall_Normal.ply");

			auto r7 = new Graphic::Mesh(*r5);
			auto r8 = new Graphic::Mesh(*r5);

			delete r5;
			delete r8;
		}

		{
			auto r1 = Graphic::Asset::Shader::LoadAsync("C:\\Users\\FREEstriker\\Desktop\\Test.shader");
			auto r2 = Graphic::Asset::Shader::LoadAsync("C:\\Users\\FREEstriker\\Desktop\\Test.shader");
			auto r3 = Graphic::Asset::Shader::LoadAsync("C:\\Users\\FREEstriker\\Desktop\\Test.shader");
			auto r4 = Graphic::Asset::Shader::LoadAsync("C:\\Users\\FREEstriker\\Desktop\\Test.shader");

			auto r5 = Graphic::Asset::Shader::Load("C:\\Users\\FREEstriker\\Desktop\\Test.shader");
			auto r6 = Graphic::Asset::Shader::Load("C:\\Users\\FREEstriker\\Desktop\\Test.shader");

			auto r7 = new Graphic::Asset::Shader(*r5);
			auto r8 = new Graphic::Asset::Shader(*r5);

			//delete r5;
			//delete r8;
		}


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

