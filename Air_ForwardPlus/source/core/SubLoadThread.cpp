#include "core/SubLoadThread.h"
#include "Graphic/GlobalInstance.h"
#include "core/LoadThread.h"
#include "Graphic/CommandPool.h"
#include "Graphic/CommandBuffer.h"
SubLoadThread::SubLoadThread()
	: _loadThread(nullptr)
	, _commandPool()
	, _commandBuffer(nullptr)
{
}
SubLoadThread::SubLoadThread(const SubLoadThread& src)
	: _loadThread(nullptr)
	, _commandPool()
	, _commandBuffer(nullptr)
{
}
SubLoadThread::SubLoadThread(LoadThread& loadThread)
	: _loadThread(&loadThread)
	, _commandPool(new Graphic::CommandPool(VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, "TransferQueue"))
	, _commandBuffer(&_commandPool->CreateCommandBuffer("TransferCommandBuffer", VK_COMMAND_BUFFER_LEVEL_PRIMARY))
{

}
SubLoadThread::~SubLoadThread()
{

}

void SubLoadThread::OnStart()
{

}

void SubLoadThread::OnRun()
{
	while (true)
	{
		std::function<void(Graphic::CommandBuffer&)> task;

		{
			std::unique_lock<std::mutex> lock(_loadThread->queue_mutex);
			_loadThread->condition.wait(lock,
				[this] { return _loadThread->stop || !_loadThread->tasks.empty(); });
			if (_loadThread->stop && _loadThread->tasks.empty())
				return;
			task = std::move(_loadThread->tasks.front());
			_loadThread->tasks.pop();
		}

		task(*_commandBuffer);
	}
}

void SubLoadThread::OnEnd()
{

}
