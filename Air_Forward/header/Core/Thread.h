#pragma once
#include <Utils/ThreadBase.h>
#include <mutex>

namespace Core
{
	class Thread final
	{
	private:
		class LogicThread final : public Utils::ThreadBase
		{
		public:
			bool _stopped;

			LogicThread();
			~LogicThread();
			void Init()override;
			void OnStart() override;
			void OnThreadStart() override;
			void OnRun() override;
			void OnEnd() override;
		};
	private:
		static LogicThread* _instance;
		Thread();
		~Thread();
	public:
		static void Init();
		static void Start();
		static void End();
		static void WaitForStartFinish();
	};
}