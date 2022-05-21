#pragma once
#include <Utils/ThreadBase.h>
#include <mutex>
#include <Core/Component/Component.h>
#include <vector>

namespace Core
{
	class Thread final
	{
	private:
		class LogicThread final : public Utils::ThreadBase
		{
		private:
			void IterateByDynamicBFS(Core::Component::Component::ComponentType targetComponentType);
			void IterateByStaticBFS(Core::Component::Component::ComponentType targetComponentType);
			std::vector<Component::Component*> IterateByStaticBFSWithRecord(Core::Component::Component::ComponentType targetComponentType);
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
		static LogicThread _logicThread;
		Thread();
		~Thread();
	public:
		static void Init();
		static void Start();
		static void End();
		static void WaitForStartFinish();
	};
}