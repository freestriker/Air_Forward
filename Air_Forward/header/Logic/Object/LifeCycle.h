#pragma once

namespace Logic
{
	namespace Object
	{
		class LifeCycle
		{
		private:
			bool _neverStarted;
			bool _neverAwaked;
		public:
			void Awake();
			void Update();
			LifeCycle();
			virtual ~LifeCycle();
			virtual void OnAwake();
			virtual void OnStart();
			virtual void OnUpdate();
			virtual void OnDestroy();

		};
	}
}
