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
	SubLoadThread();
	SubLoadThread(const SubLoadThread& src);
	SubLoadThread(LoadThread& _loadThread);
	virtual ~SubLoadThread();

private:
	std::unique_ptr<Graphic::CommandPool> _commandPool;
	Graphic::CommandBuffer* const _commandBuffer;
	LoadThread* const _loadThread;

	void OnStart()override;
	void OnRun()override;
	void OnEnd()override;

};