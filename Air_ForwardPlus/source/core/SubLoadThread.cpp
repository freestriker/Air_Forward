#include "core/SubLoadThread.h"
#include "core/LoadThread.h"
#include "Graphic/Command/CommandPool.h"
#include "Graphic/Command/CommandBuffer.h"
SubLoadThread::SubLoadThread(LoadThread& loadThread)
	: _loadThread(&loadThread)
	, _transferCommandPool(new Graphic::Command::CommandPool(VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, "TransferQueue"))
	, _transferCommandBuffer(_transferCommandPool->CreateCommandBuffer("TransferCommandBuffer", VK_COMMAND_BUFFER_LEVEL_PRIMARY))
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
		std::function<void(Graphic::Command::CommandBuffer* const)> task;

		{
			std::unique_lock<std::mutex> lock(_loadThread->_queueMutex);
			_loadThread->_queueVariable.wait(lock,
				[this] { return _loadThread->_stopped || !_loadThread->_tasks.empty(); });
			if (_loadThread->_stopped && _loadThread->_tasks.empty())
				return;
			task = std::move(_loadThread->_tasks.front());
			_loadThread->_tasks.pop();
		}

		task(_transferCommandBuffer);
	}
}

void SubLoadThread::OnEnd()
{

}
