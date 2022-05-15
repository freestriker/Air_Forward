#pragma once
#include "Utils/ThreadBase.h"
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
		class Thread final
		{
		private:
			class RenderThread final : public Utils::ThreadBase
			{
			public:
				static RenderThread* _instance;
				bool _stopped;
				Command::CommandPool* renderCommandPool;
				Command::CommandBuffer* renderCommandBuffer;
				Command::CommandPool* presentCommandPool;
				Command::CommandBuffer* presentCommandBuffer;

				std::mutex _mutex;
				std::condition_variable _readyToRenderCondition;
				bool _readyToRender;
				RenderThread();
				~RenderThread();
				void Init()override;
				void StartRender();
				void OnStart() override;
				void OnThreadStart() override;
				void OnRun() override;
				void OnEnd() override;
			};
		private:
			static RenderThread* _instance;
			Thread();
			~Thread();
		public:
			static void Init();
			static void Start();
			static void StartRender();
			static void End();
			static void WaitForStartFinish();
		};
	}
}
