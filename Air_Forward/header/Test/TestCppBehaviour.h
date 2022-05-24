#pragma once
#include "Logic/Component/Behaviour/CppBehaviour.h"
#include <string>
namespace Logic
{
	namespace Object
	{
		class GameObject;
	}
}
namespace Test
{
	class TestCppBehaviour : public Logic::Component::Behaviour::CppBehaviour
	{
	public:
		Logic::Object::GameObject* destroyObject;
		std::string name;
		TestCppBehaviour(std::string name);
		TestCppBehaviour(std::string name, Logic::Object::GameObject* destroyObject);
		TestCppBehaviour();
		~TestCppBehaviour();
		void OnAwake()override;
		void OnStart()override;
		void OnUpdate()override;
		void OnDestroy()override;
		RTTR_ENABLE(Logic::Component::Behaviour::CppBehaviour)
	};
}