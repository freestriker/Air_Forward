#include "Core/Thread.h"
#include <Core/Object/GameObject.h>
#include <Core/Component/Component.h>
#include "Core/Instance.h"
#include "Utils/Log.h"
#include "Core/Manager/ObjectFactory.h"

Core::Thread::LogicThread Core::Thread::_logicThread = Core::Thread::LogicThread();

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

	{
		go0->AddComponent(new Core::Component::Transform::Transform());
		go0->AddComponent(new Core::Component::Transform::Transform());
		go0->AddComponent(new Core::Component::Transform::Transform());
		go0->RemoveComponents("Core::Component::Transform::Transform");
	}
	{
		go0->AddComponent(new Core::Component::Transform::Transform());
		go0->AddComponent(new Core::Component::Transform::Transform());
		go0->AddComponent(new Core::Component::Transform::Transform());
		for (const auto& foundTransform : Core::Instance::rootObject.Child()->GetComponents("Core::Component::Transform::Transform"))
		{
			Core::Manager::ObjectFactory::Destroy(foundTransform);
		}
	}
	{
		go0->AddComponent(new Core::Component::Transform::Transform());
		go00->AddComponent(new Core::Component::Transform::Transform());
		go01->AddComponent(new Core::Component::Transform::Transform());
		Core::Manager::ObjectFactory::Destroy(go0);
	}


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
	_logicThread.Init();
}

void Core::Thread::Start()
{
	_logicThread.Start();
}

void Core::Thread::End()
{
	_logicThread.End();
}
void Core::Thread::WaitForStartFinish()
{
	_logicThread.WaitForStartFinish();
}