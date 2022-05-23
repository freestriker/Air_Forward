#pragma once
#include "Logic/Component/Behaviour/Behaviour.h"

namespace Logic
{
	namespace Component
	{
		namespace Behaviour
		{
			class CppBehaviour : public Logic::Component::Behaviour::Behaviour
			{
			public:
				CppBehaviour();
				~CppBehaviour();
				virtual void OnAwake()override = 0;
				virtual void OnStart()override = 0;
				virtual void OnUpdate()override = 0;
				virtual void OnDestroy()override = 0;
				RTTR_ENABLE(Logic::Component::Behaviour::Behaviour)
			};
		}
	}
}