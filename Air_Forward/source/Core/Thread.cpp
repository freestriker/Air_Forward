#include "Core/Thread.h"
#include <Core/Object/GameObject.h>
#include <Core/Component/Component.h>
#include "Core/Instance.h"
#include "Utils/Log.h"
#include "Core/Manager/ObjectFactory.h"
#include <list>

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
		go0->AddComponent(new Core::Component::Transform::Transform());
		go0->AddComponent(new Core::Component::Transform::Transform());
		go0->AddComponent(new Core::Component::Transform::Transform());
		go0->AddComponent(new Core::Component::Transform::Transform());
		go0->AddComponent(new Core::Component::Transform::Transform());
		go0->AddComponent(new Core::Component::Transform::Transform());
		auto f1 = go0->GetComponent<Core::Component::Transform::Transform>();
		auto r1 = go0->RemoveComponent<Core::Component::Transform::Transform>();
		auto f2 = go0->GetComponent("Core::Component::Transform::Transform");
		go0->RemoveComponent(f2);
		auto f3 = go0->GetComponents<Core::Component::Transform::Transform>();
		go0->RemoveComponents(std::vector<Component::Component*>({ f3[0], f3[1] }));
		auto f4 = go0->GetComponents("Core::Component::Transform::Transform");
		auto r2 = go0->RemoveComponents<Core::Component::Transform::Transform>();
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

	std::list< Core::Object::GameObject*> curGenGameObjectHeads = std::list<Core::Object::GameObject*>();
	std::list< Core::Object::GameObject*> nextGenGameObjectHeads = std::list<Core::Object::GameObject*>();
	auto& validGameObjectInIteration = Instance::_validGameObjectInIteration;
	auto& validComponentInIteration = Instance::_validComponentInIteration;

	while (!_stopped)
	{
		auto targetComponentType = Core::Component::Component::ComponentType::BEHAVIOUR;
		//Clear
		validGameObjectInIteration.clear();
		validComponentInIteration.clear();
		curGenGameObjectHeads.clear();
		nextGenGameObjectHeads.clear();

		//Init
		if (Instance::rootObject._gameObject.HaveChild())
		{
			validGameObjectInIteration.emplace(Instance::rootObject._gameObject.Child());

			curGenGameObjectHeads.emplace_back(Instance::rootObject._gameObject.Child());
		}

		while (!curGenGameObjectHeads.empty())
		{
			for (const auto& curGenGameObjectHead : curGenGameObjectHeads)
			{
				if (!validGameObjectInIteration.count(curGenGameObjectHead)) continue;

				std::vector<Core::Object::GameObject*> curGenGameObjects = std::vector<Core::Object::GameObject*>();
				{
					curGenGameObjects.emplace_back(curGenGameObjectHead);
					Object::GameObject* gameObject = curGenGameObjectHead->Brother();
					while (gameObject)
					{
						curGenGameObjects.emplace_back(gameObject);
						validGameObjectInIteration.emplace(gameObject);

						gameObject = gameObject->Brother();
					}
				}

				for (const auto& curGenGameObject : curGenGameObjects)
				{
					if (!validGameObjectInIteration.count(curGenGameObject))continue;

					//Run Components
					validComponentInIteration.clear();
					if (curGenGameObject->_typeSqueueComponentsHeadMap.count(targetComponentType))
					{
						std::vector< Component::Component*> components = std::vector< Component::Component*>();
						for (auto iterator = curGenGameObject->_typeSqueueComponentsHeadMap[targetComponentType]->GetIterator(); iterator.IsValid(); iterator++)
						{
							auto component = dynamic_cast<Component::Component*>(iterator.Node());

							validComponentInIteration.insert(component);
							components.emplace_back(component);
						}

						for (const auto& component : components)
						{
							if (validComponentInIteration.count(component)) component->Update();
						}
					}

					if (!validGameObjectInIteration.count(curGenGameObject))continue;
					if (curGenGameObject->HaveChild())
					{
						auto childHead = curGenGameObject->Child();

						nextGenGameObjectHeads.emplace_back(childHead);
						validGameObjectInIteration.emplace(childHead);
					}
				}
			}

			curGenGameObjectHeads.clear();
			std::swap(nextGenGameObjectHeads, curGenGameObjectHeads);
		}
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