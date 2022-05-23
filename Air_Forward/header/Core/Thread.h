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
			void IterateByDynamicBfs(Core::Component::Component::ComponentType targetComponentType);
			void IterateByStaticBfs(Core::Component::Component::ComponentType targetComponentType);
			void IterateByStaticBfs(Core::Component::Component::ComponentType targetComponentType, std::vector<Component::Component*>& targetComponents);
			void IterateByStaticBfs(std::vector<Core::Component::Component::ComponentType> targetComponentTypes);
			void IterateByStaticBfs(std::vector<Core::Component::Component::ComponentType> targetComponentTypes, std::vector < std::vector<Component::Component*>>& targetComponents);
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