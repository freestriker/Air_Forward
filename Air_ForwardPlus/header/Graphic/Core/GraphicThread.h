#pragma once
#include "utils/Thread.h"
#include <mutex>
#include <condition_variable>
namespace Graphic
{
	namespace Command
	{
		class CommandPool;
		class CommandBuffer;
	}
	namespace Core
	{
		class GraphicThread final : protected _Thread
		{
		private:
			static GraphicThread* _instance;
			bool _stopped;
			Command::CommandPool* renderCommandPool;
			Command::CommandBuffer* renderCommandBuffer;
			Command::CommandPool* presentCommandPool;
			Command::CommandBuffer* presentCommandBuffer;

			std::mutex _mutex;
			std::condition_variable _readyToRenderCondition;
			bool _readyToRender;
			GraphicThread();
			~GraphicThread();
			void Init()override;
			void _StartRender();
			void OnStart() override;
			void OnThreadStart() override;
			void OnRun() override;
			void OnEnd() override;
		public:
			static void InitThread();
			static void StartThread();
			static void StartRender();
			static void EndThread();
			static void WaitForThreadStartFinish();
		};
	}
}
