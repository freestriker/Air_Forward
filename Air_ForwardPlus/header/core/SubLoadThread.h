#pragma once
#include "utils/Thread.h"
#include "vulkan/vulkan_core.h"
#include <memory>
namespace Graphic
{
	namespace Command
	{
		class CommandPool;
		class CommandBuffer;
	}
}
class SubLoadThread final : public Thread
{
	friend class LoadThread;
public:


private:
	SubLoadThread(LoadThread& _loadThread);
	~SubLoadThread();

	SubLoadThread(const SubLoadThread&) = delete;
	SubLoadThread& operator=(const SubLoadThread&) = delete;
	SubLoadThread(SubLoadThread&&) = delete;
	SubLoadThread& operator=(SubLoadThread&&) = delete;
private:
	std::unique_ptr<Graphic::Command::CommandPool> _transferCommandPool;
	LoadThread* const _loadThread;
	Graphic::Command::CommandBuffer* const _transferCommandBuffer;
	void OnStart()override;
	void OnRun()override;
	void OnEnd()override;

};