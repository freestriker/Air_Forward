#pragma once
#include "Utils/ThreadBase.h"
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
#include "Utils/Log.h"

namespace Graphic
{
	namespace Command
	{
		class CommandPool;
		class CommandBuffer;
	}
}
namespace IO
{
	namespace Core
	{
		class Thread final
		{
		private:
			class SubLoadThread;
			Thread();
			~Thread();
			class LoadThread final
				: public Utils::ThreadBase
			{
				friend class SubLoadThread;
				friend class Thread;
			public:
				std::vector<SubLoadThread*> _subLoadThreads;

				bool _stopped = false;

				void Init()override;
				void OnThreadStart()override;
				void OnStart()override;
				void OnRun()override;
				void OnEnd() override;

				LoadThread();
				~LoadThread();
				LoadThread(const LoadThread&) = delete;
				LoadThread& operator=(const LoadThread&) = delete;
				LoadThread(LoadThread&&) = delete;
				LoadThread& operator=(LoadThread&&) = delete;
			};
			class SubLoadThread final
				: public Utils::ThreadBase
			{
				friend class LoadThread;
				friend class Thread;
			private:
				Graphic::Command::CommandPool* _transferCommandPool;
				Graphic::Command::CommandBuffer* _transferCommandBuffer;
			public:
				SubLoadThread();
				~SubLoadThread();

				SubLoadThread(const SubLoadThread&) = delete;
				SubLoadThread& operator=(const SubLoadThread&) = delete;
				SubLoadThread(SubLoadThread&&) = delete;
				SubLoadThread& operator=(SubLoadThread&&) = delete;

				void Init()override;
				void OnStart()override;
				void OnRun()override;
				void OnEnd()override;
			};

			static LoadThread _loadThread;
		public:
			inline static void Init();
			inline static void Start();
			inline static void End();
			inline static void WaitForStartFinish();

			static std::queue<std::function<void(Graphic::Command::CommandBuffer* const)>> _tasks;
			static std::mutex _queueMutex;
			static std::condition_variable _queueVariable;
			template<typename F, typename... Args>
			inline static auto AddTask(F&& f, Args&&... args)->std::future<typename std::invoke_result<F, Graphic::Command::CommandBuffer* const, Args...>::type>;
		};
	}
}

template<typename F, typename ...Args>
inline auto IO::Core::Thread::AddTask(F&& f, Args && ...args) -> std::future<typename std::invoke_result<F, Graphic::Command::CommandBuffer* const, Args...>::type>
{
	using return_type = typename std::invoke_result<F, Graphic::Command::CommandBuffer* const, Args...>::type;

	auto task = std::make_shared<std::packaged_task<return_type(Graphic::Command::CommandBuffer* const)> >(
		std::bind(std::forward<F>(f), std::placeholders::_1, std::forward<Args>(args)...)
		);

	std::future<return_type> res = task->get_future();
	{
		std::unique_lock<std::mutex> lock(_queueMutex);

		// don't allow enqueueing after stopping the pool
		Utils::Log::Exception("Can not add new load task when loadthread stopped.", _loadThread._stopped);

		_tasks.emplace([task](Graphic::Command::CommandBuffer* const transferCommandBuffer) { (*task)(transferCommandBuffer); });
	}
	_queueVariable.notify_one();
	return res;
}

inline void IO::Core::Thread::Init()
{
	_loadThread.Init();
}

inline void IO::Core::Thread::Start()
{
	_loadThread.Start();
}

inline void IO::Core::Thread::End()
{
	_loadThread.End();
}
inline void IO::Core::Thread::WaitForStartFinish()
{
	_loadThread.WaitForStartFinish();
}