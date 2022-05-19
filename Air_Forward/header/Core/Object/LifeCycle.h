#pragma once

namespace Core
{
	namespace Object
	{
		class LifeCycle
		{
		private:
			bool _neverStarted;
			void Awake();
			void Update();
		public:
			LifeCycle();
			virtual ~LifeCycle();
			virtual void OnAwake();
			virtual void OnStart();
			virtual void OnUpdate();
			virtual void OnDestory();

		};
	}
}
