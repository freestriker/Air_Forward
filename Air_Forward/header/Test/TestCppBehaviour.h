#pragma once
#include "Core/Component/Behaviour/CppBehaviour.h"
#include <string>
namespace Core
{
	namespace Object
	{
		class GameObject;
	}
}
namespace Test
{
	class TestCppBehaviour : public Core::Component::Behaviour::CppBehaviour
	{
	public:
		Core::Object::GameObject* destroyObject;
		std::string name;
		TestCppBehaviour(std::string name);
		TestCppBehaviour(std::string name, Core::Object::GameObject* destroyObject);
		TestCppBehaviour();
		~TestCppBehaviour();
		void OnAwake()override;
		void OnStart()override;
		void OnUpdate()override;
		void OnDestroy()override;
		RTTR_ENABLE(Core::Component::Behaviour::CppBehaviour)
	};
}