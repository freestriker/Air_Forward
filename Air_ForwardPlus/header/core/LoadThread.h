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

class LoadThread : public Thread
{
	friend class SubLoadThread;
private:
	std::queue< std::function<void(VkCommandBuffer)> > tasks;
	std::vector<SubLoadThread> subLoadThreads;
	std::mutex queue_mutex;
	std::condition_variable condition;
	bool stop = false;

	void OnStart()override;
	void OnRun()override;
	void OnEnd() override;
	LoadThread();
	~LoadThread();


public:
	static LoadThread* const instance;
	template<typename F, typename... Args>
	auto AddTask(F&& f, Args&&... args)
		->std::future<typename std::invoke_result<F(VkCommandBuffer, Args...)>::type>;
};

template<typename F, typename ...Args>
inline auto LoadThread::AddTask(F&& f, Args && ...args) -> std::future<typename std::invoke_result<F(VkCommandBuffer, Args...)>::type>
{
	using return_type = typename std::invoke_result<F(VkCommandBuffer, Args...)>::type;

	auto task = std::make_shared< std::packaged_task<return_type(VkCommandBuffer)> >(
		std::bind(std::forward<F>(f), std::placeholders::_1, std::forward<Args>(args)...)
		);

	std::future<return_type> res = task->get_future();
	{
		std::unique_lock<std::mutex> lock(queue_mutex);

		// don't allow enqueueing after stopping the pool
		if (stop)
			throw std::runtime_error("enqueue on stopped ThreadPool");

		tasks.emplace([task](VkCommandBuffer commandBuffer) { (*task)(commandBuffer); });
	}
	condition.notify_one();
	return res;
}
