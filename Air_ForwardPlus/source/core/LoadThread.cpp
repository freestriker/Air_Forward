#include "core/LoadThread.h"
#include <iostream>
#include <chrono> 
#include <core/SubLoadThread.h>
#include "Graphic/Asset/Texture2D.h"
#include "Graphic/CommandBuffer.h"
#include "core/AssetUtils.h"
#include "Graphic/Asset/Mesh.h"
#include "Graphic/Asset/Shader.h"
#include "Graphic/Material.h"
#include "Graphic/Asset/UniformBuffer.h"
#include <glm/glm.hpp>

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
			auto r1 = Graphic::Texture2D::LoadAsync("..\\Asset\\Texture\\Wall.png");
			auto r2 = Graphic::Texture2D::LoadAsync("..\\Asset\\Texture\\Wall.png");
			auto r3 = Graphic::Texture2D::LoadAsync("..\\Asset\\Texture\\Wall.png");
			auto r4 = Graphic::Texture2D::LoadAsync("..\\Asset\\Texture\\Wall.png");

			auto r5 = Graphic::Texture2D::Load("..\\Asset\\Texture\\Wall.png");
			auto r6 = Graphic::Texture2D::Load("..\\Asset\\Texture\\Wall.png");

			auto r7 = new Graphic::Texture2D(*r5);
			auto r8 = new Graphic::Texture2D(*r5);

			delete r5;
			delete r8;
		}

		{
			auto r1 = Graphic::Mesh::LoadAsync("..\\Asset\\Mesh\\Flat_Wall_Normal.ply");
			auto r2 = Graphic::Mesh::LoadAsync("..\\Asset\\Mesh\\Flat_Wall_Normal.ply");
			auto r3 = Graphic::Mesh::LoadAsync("..\\Asset\\Mesh\\Flat_Wall_Normal.ply");
			auto r4 = Graphic::Mesh::LoadAsync("..\\Asset\\Mesh\\Flat_Wall_Normal.ply");

			auto r5 = Graphic::Mesh::Load("..\\Asset\\Mesh\\Flat_Wall_Normal.ply");
			auto r6 = Graphic::Mesh::Load("..\\Asset\\Mesh\\Flat_Wall_Normal.ply");

			auto r7 = new Graphic::Mesh(*r5);
			auto r8 = new Graphic::Mesh(*r5);

			delete r5;
			delete r8;
		}

		{
			auto r1 = Graphic::Asset::Shader::LoadAsync("..\\Asset\\Shader\\Test.shader");
			auto r2 = Graphic::Asset::Shader::LoadAsync("..\\Asset\\Shader\\Test.shader");
			auto r3 = Graphic::Asset::Shader::LoadAsync("..\\Asset\\Shader\\Test.shader");
			auto r4 = Graphic::Asset::Shader::LoadAsync("..\\Asset\\Shader\\Test.shader");

			auto r5 = Graphic::Asset::Shader::Load("..\\Asset\\Shader\\Test.shader");
			auto r6 = Graphic::Asset::Shader::Load("..\\Asset\\Shader\\Test.shader");

			auto r7 = new Graphic::Asset::Shader(*r5);
			auto r8 = new Graphic::Asset::Shader(*r5);

			//delete r5;
			//delete r8;
		}

		{
			glm::mat4 modelMatrix = glm::mat4(1.0f);

			auto r1 = Graphic::Asset::Shader::LoadAsync("..\\Asset\\Shader\\Test.shader");
			auto r2 = Graphic::Texture2D::LoadAsync("..\\Asset\\Texture\\Wall.png");
			auto r3 = new Graphic::Asset::UniformBuffer(sizeof(modelMatrix), VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
			r3->WriteBuffer(&modelMatrix, sizeof(modelMatrix));

			auto material = new Graphic::Material(r1.get());
			material->SetTexture2D("testTexture2D", r2.get());
			auto texture2d = material->GetTexture2D("testTexture2D");
			delete material;
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

