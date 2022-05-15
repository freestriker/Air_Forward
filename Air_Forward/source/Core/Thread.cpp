#include "Core/Thread.h"
#include <Core/Object/GameObject.h>
#include <Core/Component/Component.h>
#include "Core/Instance.h"
#include "Utils/Log.h"

Core::Thread::LogicThread* Core::Thread::_instance = nullptr;

Core::Thread::LogicThread::LogicThread()
	:_stopped(true)
{
}

Core::Thread::LogicThread::~LogicThread()
{
}

void Core::Thread::LogicThread::Init()
{
}

void Core::Thread::LogicThread::OnStart()
{
	_stopped = false;
}

void Core::Thread::LogicThread::OnThreadStart()
{
}

void Core::Thread::LogicThread::OnRun()
{
	Core::Object::GameObject* go0 = new Core::Object::GameObject("go0");
	Core::Instance::rootObject.AddChild(go0);

	Core::Object::GameObject* go1 = new Core::Object::GameObject("go1");
	Core::Instance::rootObject.AddChild(go1);

	Core::Object::GameObject* go2 = new Core::Object::GameObject("go2");
	Core::Instance::rootObject.AddChild(go2);

	Core::Object::GameObject* go00 = new Core::Object::GameObject("go00");
	go0->AddChild(go00);

	Core::Object::GameObject* go01 = new Core::Object::GameObject("go01");
	go0->AddChild(go01);

	Core::Component::Transform::Transform* testTransform = new Core::Component::Transform::Transform();
	go0->AddComponent(testTransform);

	auto foundTransforms = Core::Instance::rootObject.Child()->GetComponents("Core::Component::Transform::Transform");

	while (!_stopped)
	{

	}
}

void Core::Thread::LogicThread::OnEnd()
{
	_stopped = true;
}

Core::Thread::Thread()
{
}
Core::Thread::~Thread()
{
}

void Core::Thread::Init()
{
	_instance = new Core::Thread::LogicThread();
	_instance->Init();
}

void Core::Thread::Start()
{
	_instance->Start();
}

void Core::Thread::End()
{
	_instance->End();
}
void Core::Thread::WaitForStartFinish()
{
	_instance->WaitForStartFinish();
}