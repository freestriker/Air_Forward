#include "Test/TestCppBehaviour.h"
#include "Utils/Log.h"
#include <rttr/registration>
RTTR_REGISTRATION
{
	rttr::registration::class_<Test::TestCppBehaviour>("Test::TestCppBehaviour");
}

Test::TestCppBehaviour::TestCppBehaviour(std::string name)
	: name(name)
{
}

Test::TestCppBehaviour::TestCppBehaviour()
	: TestCppBehaviour("Default TestCppBehaviour")
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
}

void Test::TestCppBehaviour::OnDestroy()
{
	Utils::Log::Message(name + " OnDestroy().");
}
