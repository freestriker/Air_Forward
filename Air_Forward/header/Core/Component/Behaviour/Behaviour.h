#pragma once
#include "Core/Component/Component.h"

namespace Core
{
	namespace Component
	{
		namespace Behaviour
		{
			class Behaviour: public Core::Component::Component
			{
			public:
				Behaviour();
				~Behaviour();
				virtual void OnAwake()override = 0;
				virtual void OnStart()override = 0;
				virtual void OnUpdate()override = 0;
				virtual void OnDestroy()override = 0;
			};
		}
	}
}