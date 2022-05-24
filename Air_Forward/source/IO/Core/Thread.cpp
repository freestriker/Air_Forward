#include "IO/Core/Thread.h"
#include "IO/Core/Instance.h"
#include <iostream>
#include <chrono> 
#include "Graphic/Command/CommandBuffer.h"
#include <glm/glm.hpp>
#include "Graphic/Command/CommandPool.h"
#include "Graphic/Command/CommandBuffer.h"

IO::Core::Thread::LoadThread IO::Core::Thread::_loadThread = IO::Core::Thread::LoadThread();
std::queue<std::function<void(Graphic::Command::CommandBuffer* const)>> IO::Core::Thread::_tasks = std::queue<std::function<void(Graphic::Command::CommandBuffer* const)>>();
std::mutex IO::Core::Thread::_queueMutex = std::mutex();
std::condition_variable IO::Core::Thread::_queueVariable = std::condition_variable();

void IO::Core::Thread::LoadThread::Init()
{
	_stopped = true;
	_subLoadThreads.emplace_back(new SubLoadThread());
	_subLoadThreads.emplace_back(new SubLoadThread());
	_subLoadThreads.emplace_back(new SubLoadThread());
	_subLoadThreads.emplace_back(new SubLoadThread());
	for (auto& subThread : _subLoadThreads)
	{
		subThread->Init();
	}
}


void IO::Core::Thread::LoadThread::OnStart()
{
	_stopped = false;
}

void IO::Core::Thread::LoadThread::OnThreadStart()
{
	for (auto& subLoadThread : _subLoadThreads)
	{
		subLoadThread->Start();
	}
}

void IO::Core::Thread::LoadThread::OnRun()
{
	while (!_stopped)
	{
		std::this_thread::sleep_for(std::chrono::seconds(1));

		Instance::assetManager.Collect();
	}
}
void IO::Core::Thread::LoadThread::OnEnd()
{
	_stopped = true;

	for (auto& subLoadThread : _subLoadThreads)
	{
		subLoadThread->End();
	}
}

IO::Core::Thread::LoadThread::LoadThread()
	: ThreadBase()
	, _subLoadThreads()
	, _stopped(true)
{
}

IO::Core::Thread::LoadThread::~LoadThread()
{

}


IO::Core::Thread::SubLoadThread::SubLoadThread()
	: _transferCommandPool(nullptr)
	, _transferCommandBuffer(nullptr)
{
}

IO::Core::Thread::SubLoadThread::~SubLoadThread()
{

}

void IO::Core::Thread::SubLoadThread::Init()
{
	_transferCommandPool = new Graphic::Command::CommandPool(VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, "TransferQueue");
	_transferCommandBuffer = _transferCommandPool->CreateCommandBuffer("TransferCommandBuffer", VK_COMMAND_BUFFER_LEVEL_PRIMARY);

}
void IO::Core::Thread::SubLoadThread::OnStart()
{

}

void IO::Core::Thread::SubLoadThread::OnRun()
{
	while (true)
	{
		std::function<void(Graphic::Command::CommandBuffer* const)> task;

		{
			std::unique_lock<std::mutex> lock(_queueMutex);
			_queueVariable.wait(lock, [this] { return _loadThread._stopped || !_tasks.empty(); });
			if (_loadThread._stopped && _tasks.empty())
			{
				return;
			}
			task = std::move(_tasks.front());
			_tasks.pop();
		}

		task(_transferCommandBuffer);
	}
}

void IO::Core::Thread::SubLoadThread::OnEnd()
{

}