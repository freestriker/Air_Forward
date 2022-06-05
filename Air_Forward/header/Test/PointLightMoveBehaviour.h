#pragma once
#include "Logic/Component/Behaviour/CppBehaviour.h"
#include <string>
#include <glm/glm.hpp>
namespace Test
{
	class PointLightMoveBehaviour : public Logic::Component::Behaviour::CppBehaviour
	{
	private:
		float _eulerAngularVelocity;
	public:
		PointLightMoveBehaviour(float eulerAngularVelocity);
		~PointLightMoveBehaviour();
		void OnAwake()override;
		void OnStart()override;
		void OnUpdate()override;
		void OnDestroy()override;

		RTTR_ENABLE(Logic::Component::Behaviour::CppBehaviour)
	};
}
