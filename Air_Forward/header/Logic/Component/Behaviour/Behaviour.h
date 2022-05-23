#pragma once
#include "Logic/Component/Component.h"

namespace Logic
{
	namespace Component
	{
		namespace Behaviour
		{
			class Behaviour: public Logic::Component::Component
			{
			public:
				Behaviour();
				~Behaviour();
				virtual void OnAwake()override = 0;
				virtual void OnStart()override = 0;
				virtual void OnUpdate()override = 0;
				virtual void OnDestroy()override = 0;

				RTTR_ENABLE(Logic::Component::Component)
			};
		}
	}
}