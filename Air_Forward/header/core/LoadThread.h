#pragma once
#include "utils/ThreadBase.h"
#include <vector>
#include <queue>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <future>
#include <vulkan/vulkan_core.h>
#include <memory>
#include <map>
class AssetManager;
namespace Graphic
{
	namespace Command
	{
		class CommandBuffer;
	}
}
class LoadThread : public ThreadBase
{
	friend class SubLoadThread;
private:
	std::queue<std::function<void(Graphic::Command::CommandBuffer* const)>> _tasks;
	std::vector<SubLoadThread*> _subLoadThreads;


	std::mutex _queueMutex;
	std::condition_variable _queueVariable;
	bool _stopped = false;

	void OnStart()override;
	void OnRun()override;
	void OnEnd() override;

	LoadThread();
	~LoadThread();
	LoadThread(const LoadThread&) = delete;
	LoadThread& operator=(const LoadThread&) = delete;
	LoadThread(LoadThread&&) = delete;
	LoadThread& operator=(LoadThread&&) = delete;


public:
	static LoadThread* const instance;
	std::unique_ptr<AssetManager> assetManager;
	void Init()override;
	template<typename F, typename... Args>
	auto AddTask(F&& f, Args&&... args) -> std::future<typename std::invoke_result<F, Graphic::Command::CommandBuffer* const, Args...>::type>;
};

template<typename F, typename ...Args>
auto LoadThread::AddTask(F&& f, Args && ...args) -> std::future<typename std::invoke_result<F, Graphic::Command::CommandBuffer* const, Args...>::type>
{
	using return_type = typename std::invoke_result<F, Graphic::Command::CommandBuffer* const, Args...>::type;

	auto task = std::make_shared< std::packaged_task<return_type(Graphic::Command::CommandBuffer* const)> >(
		std::bind(std::forward<F>(f), std::placeholders::_1, std::forward<Args>(args)...)
		);

	std::future<return_type> res = task->get_future();
	{
		std::unique_lock<std::mutex> lock(_queueMutex);

		// don't allow enqueueing after stopping the pool
		if (_stopped)
			std::cerr << "enqueue on stopped ThreadPool";

		_tasks.emplace([task](Graphic::Command::CommandBuffer* const transferCommandBuffer) { (*task)(transferCommandBuffer); });
	}
	_queueVariable.notify_one();
	return res;
}
