#pragma once
#include "Core/Component/Behaviour/CppBehaviour.h"
#include <string>
namespace Test
{
	class TestCppBehaviour : public Core::Component::Behaviour::CppBehaviour
	{
	public:
		std::string name;
		TestCppBehaviour(std::string name);
		TestCppBehaviour();
		~TestCppBehaviour();
		void OnAwake()override;
		void OnStart()override;
		void OnUpdate()override;
		void OnDestroy()override;
	};
}