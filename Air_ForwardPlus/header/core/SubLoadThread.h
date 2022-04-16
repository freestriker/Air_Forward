#pragma once
#include "utils/Thread.h"
#include "vulkan/vulkan_core.h"
#include <memory>
namespace Graphic
{
	class CommandPool;
	class CommandBuffer;
}
class SubLoadThread: public Thread
{
	friend class LoadThread;
public:
	SubLoadThread(LoadThread& _loadThread);
	virtual ~SubLoadThread();

	SubLoadThread(const SubLoadThread&) = delete;
	SubLoadThread& operator=(const SubLoadThread&) = delete;
	SubLoadThread(SubLoadThread&&) = delete;
	SubLoadThread& operator=(SubLoadThread&&) = delete;

private:
	std::unique_ptr<Graphic::CommandPool> _commandPool;
	LoadThread* const _loadThread;
	Graphic::CommandBuffer* const _commandBuffer;
	void OnStart()override;
	void OnRun()override;
	void OnEnd()override;

};