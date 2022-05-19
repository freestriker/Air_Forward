#pragma once
#include "Core/Component/Behaviour/Behaviour.h"
namespace Core
{
	namespace Component
	{
		namespace Behaviour
		{
			class CppBehaviour : public Core::Component::Behaviour::Behaviour
			{
			public:
				CppBehaviour();
				~CppBehaviour();
				virtual void OnAwake()override = 0;
				virtual void OnStart()override = 0;
				virtual void OnUpdate()override = 0;
				virtual void OnDestroy()override = 0;
			};
		}
	}
}