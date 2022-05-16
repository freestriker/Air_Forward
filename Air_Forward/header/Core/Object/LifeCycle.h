#pragma once
#include <rttr/type>

namespace Core
{
	namespace Object
	{
		class LifeCycle
		{
		protected:
			bool _active;
			LifeCycle();
			virtual ~LifeCycle();
			virtual void OnAwake();
			virtual void OnStart();
			virtual void OnUpdate();
			virtual void OnDestory();
			virtual void OnEnable();
			virtual void OnDisable();
			virtual bool OnCheckValid();
		public:
			bool Active();
			void SetActive(bool active);

			RTTR_ENABLE()
		};
	}
}
