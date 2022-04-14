#include "core/SubLoadThread.h"
#include "Graphic/GlobalInstance.h"
#include "core/LoadThread.h"
SubLoadThread::SubLoadThread()
	: _commandPool(VK_NULL_HANDLE)
	, _commandBuffer(VK_NULL_HANDLE)
	, _loadThread(nullptr)
{
}
SubLoadThread::~SubLoadThread()
{
}

void SubLoadThread::PopulateLoadThread(LoadThread* loadThread)
{
	_loadThread = loadThread;
}

void SubLoadThread::OnStart()
{
	Graphic::GlobalInstance::CreateCommandPool(VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, "TransferQueue", _commandPool);
	Graphic::GlobalInstance::CreateCommandBuffer(_commandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, _commandBuffer);
}

void SubLoadThread::OnRun()
{
	while (true)
	{
		std::function<void(VkCommandBuffer)> task;

		{
			std::unique_lock<std::mutex> lock(_loadThread->queue_mutex);
			_loadThread->condition.wait(lock,
				[this] { return _loadThread->stop || !_loadThread->tasks.empty(); });
			if (_loadThread->stop && _loadThread->tasks.empty())
				return;
			task = std::move(_loadThread->tasks.front());
			_loadThread->tasks.pop();
		}

		task(_commandBuffer);
	}
}

void SubLoadThread::OnEnd()
{
}
