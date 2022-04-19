#pragma once
#include "utils/Thread.h"
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
namespace Graphic
{
	class CommandBuffer;
}
class AssetInstanceManager
{
public:
	class AssetInstanceWarp
	{
	public:
		std::string path;
		uint32_t refCount;
		void* assetInstance;
	};
	std::mutex mutex;
private:
	std::map<std::string, AssetInstanceWarp> _warps;

public:
	AssetInstanceManager();
	~AssetInstanceManager();
	void AddInstance(std::string path, void* assetInstance);
	void* GetInstance(std::string path);
	bool ContainsInstance(std::string path);
	void RecycleInstance(std::string path);
};
class LoadThread : public Thread
{
	friend class SubLoadThread;
private:
	std::queue<std::function<void(Graphic::CommandBuffer* const, Graphic::CommandBuffer* const)>> _tasks;
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
	AssetInstanceManager assetInstanceManager;
	void Init()override;
	template<typename F, typename... Args>
	auto AddTask(F&& f, Args&&... args) -> std::future<typename std::invoke_result<F, Graphic::CommandBuffer* const, Graphic::CommandBuffer* const, Args...>::type>;
};

template<typename F, typename ...Args>
auto LoadThread::AddTask(F&& f, Args && ...args) -> std::future<typename std::invoke_result<F, Graphic::CommandBuffer* const, Graphic::CommandBuffer* const, Args...>::type>
{
	using return_type = typename std::invoke_result<F, Graphic::CommandBuffer* const, Graphic::CommandBuffer* const, Args...>::type;

	auto task = std::make_shared< std::packaged_task<return_type(Graphic::CommandBuffer* const, Graphic::CommandBuffer* const)> >(
		std::bind(std::forward<F>(f), std::placeholders::_1, std::placeholders::_2, std::forward<Args>(args)...)
		);

	std::future<return_type> res = task->get_future();
	{
		std::unique_lock<std::mutex> lock(_queueMutex);

		// don't allow enqueueing after stopping the pool
		if (_stopped)
			throw std::runtime_error("enqueue on stopped ThreadPool");

		_tasks.emplace([task](Graphic::CommandBuffer* const transferCommandBuffer, Graphic::CommandBuffer* const graphicCommandBuffer) { (*task)(transferCommandBuffer, graphicCommandBuffer); });
	}
	_queueVariable.notify_one();
	return res;
}
