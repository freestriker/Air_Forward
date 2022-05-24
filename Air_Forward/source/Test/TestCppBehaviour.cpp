#include "Test/TestCppBehaviour.h"
#include "Utils/Log.h"
#include <rttr/registration>
#include "Logic/Manager/ObjectFactory.h"
RTTR_REGISTRATION
{
	rttr::registration::class_<Test::TestCppBehaviour>("Test::TestCppBehaviour");
}

Test::TestCppBehaviour::TestCppBehaviour(std::string name)
	: TestCppBehaviour(name, nullptr)
{
}
Test::TestCppBehaviour::TestCppBehaviour(std::string name, Logic::Object::GameObject* destroyObject)
	: name(name)
	, destroyObject(destroyObject)
{
}

Test::TestCppBehaviour::TestCppBehaviour()
	: TestCppBehaviour("Default TestCppBehaviour", nullptr)
{
}

Test::TestCppBehaviour::~TestCppBehaviour()
{
}

void Test::TestCppBehaviour::OnAwake()
{
	Utils::Log::Message(name + " OnAwake().");
}

void Test::TestCppBehaviour::OnStart()
{
	Utils::Log::Message(name + " OnStart().");
}

void Test::TestCppBehaviour::OnUpdate()
{
	Utils::Log::Message(name + " OnUpdate().");
	if (destroyObject)
	{
		Logic::Manager::ObjectFactory::Destroy(destroyObject);
		destroyObject = nullptr;
	}
}

void Test::TestCppBehaviour::OnDestroy()
{
	Utils::Log::Message(name + " OnDestroy().");
}
