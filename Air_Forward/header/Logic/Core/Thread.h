#pragma once
#include <Utils/ThreadBase.h>
#include <mutex>
#include <Logic/Component/Component.h>
#include <vector>

namespace Logic
{
	namespace Core
	{
		class Thread final
		{
		private:
			class LogicThread final : public Utils::ThreadBase
			{
			private:
				void IterateByDynamicBfs(Logic::Component::Component::ComponentType targetComponentType);
				void IterateByStaticBfs(Logic::Component::Component::ComponentType targetComponentType);
				void IterateByStaticBfs(Logic::Component::Component::ComponentType targetComponentType, std::vector<Component::Component*>& targetComponents);
				void IterateByStaticBfs(std::vector<Logic::Component::Component::ComponentType> targetComponentTypes);
				void IterateByStaticBfs(std::vector<Logic::Component::Component::ComponentType> targetComponentTypes, std::vector < std::vector<Component::Component*>>& targetComponents);
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
}
