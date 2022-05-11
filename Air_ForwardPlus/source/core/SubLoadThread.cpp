#include "core/SubLoadThread.h"
#include "Graphic/GlobalInstance.h"
#include "core/LoadThread.h"
#include "Graphic/CommandPool.h"
#include "Graphic/CommandBuffer.h"
SubLoadThread::SubLoadThread(LoadThread& loadThread)
	: _loadThread(&loadThread)
	, _transferCommandPool(new Graphic::CommandPool(VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, "TransferQueue"))
	, _transferCommandBuffer(_transferCommandPool->CreateCommandBuffer("TransferCommandBuffer", VK_COMMAND_BUFFER_LEVEL_PRIMARY))
	, _graphiccCommandPool(new Graphic::CommandPool(VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, "TransferDstQueue"))
	, _transferDstCommandBuffer(_graphiccCommandPool->CreateCommandBuffer("TransferCommandBuffer", VK_COMMAND_BUFFER_LEVEL_PRIMARY))
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
		std::function<void(Graphic::CommandBuffer* const, Graphic::CommandBuffer* const)> task;

		{
			std::unique_lock<std::mutex> lock(_loadThread->_queueMutex);
			_loadThread->_queueVariable.wait(lock,
				[this] { return _loadThread->_stopped || !_loadThread->_tasks.empty(); });
			if (_loadThread->_stopped && _loadThread->_tasks.empty())
				return;
			task = std::move(_loadThread->_tasks.front());
			_loadThread->_tasks.pop();
		}

		task(_transferCommandBuffer, _transferDstCommandBuffer);
	}
}

void SubLoadThread::OnEnd()
{

}
