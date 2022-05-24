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
#include "Graphic/Command/CommandPool.h"
#include "Graphic/Command/CommandBuffer.h"

namespace Logic
{
	namespace Component
	{
		namespace Camera
		{
			class Camera;
		}
		namespace Renderer
		{
			class Renderer;
		}
	}
}
namespace Graphic
{
	namespace Core
	{
		class Thread final
		{
		private:
			class RenderThread;
			class SubRenderThread final : public Utils::ThreadBase
			{
				friend class RenderThread;
				friend class Thread;
			private:
				Graphic::Command::CommandPool* _commandPool;
				RenderThread* _parentRenderThread;

				SubRenderThread(const SubRenderThread&) = delete;
				SubRenderThread& operator=(const SubRenderThread&) = delete;
				SubRenderThread(SubRenderThread&&) = delete;
				SubRenderThread& operator=(SubRenderThread&&) = delete;
			public:
				SubRenderThread(RenderThread* renderThread);
				~SubRenderThread();

				void RestCommandPool();

				void Init()override;
				void OnStart()override;
				void OnThreadStart()override;
				void OnRun()override;
				void OnEnd()override;
			};

			class RenderThread final : public Utils::ThreadBase
			{
			public:
				bool _stopped;
				std::vector< SubRenderThread*> subRenderThreads;

				RenderThread();
				~RenderThread();
				void Init()override;
				void OnStart() override;
				void OnThreadStart() override;
				void OnRun() override;
				void OnEnd() override;

				std::queue<std::function<void(Graphic::Command::CommandPool*)>> _tasks;
				std::mutex _queueMutex;
				std::condition_variable _queueVariable;
				template<typename F, typename... Args>
				std::future<Graphic::Command::CommandBuffer*> AddTask(F&& f, Args&&... args);

				static Graphic::Command::CommandBuffer* RenderOpaque(Graphic::Command::CommandPool* commandPool, Logic::Component::Camera::Camera* camera, std::multimap<float, Logic::Component::Renderer::Renderer*>& renderers);
			};
		private:
			static RenderThread _renderThread;
			Thread();
			~Thread();
		public:
			static void Init();
			static void Start();
			static void End();
			static void WaitForStartFinish();
		};

	}
}

template<typename F, typename ...Args>
std::future<Graphic::Command::CommandBuffer*> Graphic::Core::Thread::RenderThread::AddTask(F&& f, Args && ...args)
{
	auto task = std::make_shared<std::packaged_task<Graphic::Command::CommandBuffer* (Graphic::Command::CommandPool*)> >(
		std::bind(std::forward<F>(f), std::placeholders::_1, std::forward<Args>(args)...)
		);

	std::future<Graphic::Command::CommandBuffer*> res = task->get_future();
	{
		std::unique_lock<std::mutex> lock(_queueMutex);

		// don't allow enqueueing after stopping the pool
		Utils::Log::Exception("Can not add new render task when renderthread stopped.", _stopped);

		_tasks.emplace([task](Graphic::Command::CommandPool* renderCommandPool) { (*task)(renderCommandPool); });
	}
	_queueVariable.notify_one();
	return res;

}
