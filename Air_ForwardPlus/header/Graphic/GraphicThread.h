#pragma once
#include "utils/Thread.h"
#include <mutex>
#include <condition_variable>
namespace Graphic
{
	class CommandPool;
	class CommandBuffer;
	class GraphicThread final : public Thread
	{
	public:
		static GraphicThread* const instance;
	private:
		bool _stopped;
		CommandPool* renderCommandPool;
		CommandBuffer* renderCommandBuffer;
		CommandPool* presentCommandPool;
		CommandBuffer* presentCommandBuffer;

		std::mutex _mutex;
		std::condition_variable _readyToRenderCondition;
		bool _readyToRender;
	public:
		GraphicThread();
		~GraphicThread();
		void Init()override;
		void StartRender();
	private:
		void OnStart() override;
		void OnThreadStart() override;
		void OnRun() override;
		void OnEnd() override;
	};
}
