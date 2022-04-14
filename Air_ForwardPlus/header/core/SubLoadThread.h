#pragma once
#include "utils/Thread.h"
#include "vulkan/vulkan_core.h"

class SubLoadThread: public Thread
{
	friend class LoadThread;
public:
	SubLoadThread();
	virtual ~SubLoadThread();

private:
	VkCommandPool _commandPool;
	VkCommandBuffer _commandBuffer;
	LoadThread* _loadThread;

	void PopulateLoadThread(LoadThread* loadThread);
	void OnStart()override;
	void OnRun()override;
	void OnEnd()override;
};