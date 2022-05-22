#include "Core/Thread.h"
#include <Core/Object/GameObject.h>
#include "Core/Instance.h"
#include "Utils/Log.h"
#include "Core/Manager/ObjectFactory.h"
#include <list>
#include "Test/TestCppBehaviour.h"
#include "Graphic/Core/Instance.h"
#include <string>
#include "Core/Component/Camera/OrthographicCamera.h"
#include "Core/Component/Renderer/MeshRenderer.h"
#include "Test/RenderTestBehaviour.h"

Core::Thread::LogicThread Core::Thread::_logicThread = Core::Thread::LogicThread();

void Core::Thread::LogicThread::IterateByDynamicBFS(Core::Component::Component::ComponentType targetComponentType)
{
	Utils::Log::Message("Core::Thread::LogicThread start iterate " + std::to_string(static_cast<int>(targetComponentType)) + " by dynamic BFS.");
	std::list< Core::Object::GameObject*> curGenGameObjectHeads = std::list<Core::Object::GameObject*>();
	std::list< Core::Object::GameObject*> nextGenGameObjectHeads = std::list<Core::Object::GameObject*>();
	auto& validGameObjectInIteration = Instance::_validGameObjectInIteration;
	auto& validComponentInIteration = Instance::_validComponentInIteration;

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
						auto component = static_cast<Component::Component*>(iterator.Node());

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

	Utils::Log::Message("Core::Thread::LogicThread finish iterate " + std::to_string(static_cast<int>(targetComponentType)) + " by dynamic BFS.");

}

void Core::Thread::LogicThread::IterateByStaticBFS(Core::Component::Component::ComponentType targetComponentType)
{
	Utils::Log::Message("Core::Thread::LogicThread start iterate " + std::to_string(static_cast<int>(targetComponentType)) + " by static BFS.");

	std::list< Core::Object::GameObject*> curGenGameObjectHeads = std::list<Core::Object::GameObject*>();
	std::list< Core::Object::GameObject*> nextGenGameObjectHeads = std::list<Core::Object::GameObject*>();

	//Init
	if (Instance::rootObject._gameObject.HaveChild())
	{
		curGenGameObjectHeads.emplace_back(Instance::rootObject._gameObject.Child());
	}

	while (!curGenGameObjectHeads.empty())
	{
		for (const auto& curGenGameObjectHead : curGenGameObjectHeads)
		{
			Object::GameObject* curGenGameObject = curGenGameObjectHead;
			while (curGenGameObject)
			{
				//Update components
				if (curGenGameObject->_typeSqueueComponentsHeadMap.count(targetComponentType))
				{
					for (auto iterator = curGenGameObject->_typeSqueueComponentsHeadMap[targetComponentType]->GetIterator(); iterator.IsValid(); iterator++)
					{
						auto component = static_cast<Component::Component*>(iterator.Node());

						component->Update();
					}
				}

				//Add next gen GameObject
				if (curGenGameObject->HaveChild())
				{
					auto childHead = curGenGameObject->Child();

					nextGenGameObjectHeads.emplace_back(childHead);
				}

				curGenGameObject = curGenGameObject->Brother();
			}

		}

		curGenGameObjectHeads.clear();
		std::swap(nextGenGameObjectHeads, curGenGameObjectHeads);
	}

	Utils::Log::Message("Core::Thread::LogicThread finish iterate " + std::to_string(static_cast<int>(targetComponentType)) + " by static BFS.");
}

std::vector<Core::Component::Component*> Core::Thread::LogicThread::IterateByStaticBFSWithRecord(Core::Component::Component::ComponentType targetComponentType)
{
	Utils::Log::Message("Core::Thread::LogicThread start iterate " + std::to_string(static_cast<int>(targetComponentType)) + " by static BFS with record.");

	std::vector<Core::Component::Component*> targetComponents = std::vector<Core::Component::Component*>();

	std::list< Core::Object::GameObject*> curGenGameObjectHeads = std::list<Core::Object::GameObject*>();
	std::list< Core::Object::GameObject*> nextGenGameObjectHeads = std::list<Core::Object::GameObject*>();

	//Init
	if (Instance::rootObject._gameObject.HaveChild())
	{
		curGenGameObjectHeads.emplace_back(Instance::rootObject._gameObject.Child());
	}

	while (!curGenGameObjectHeads.empty())
	{
		for (const auto& curGenGameObjectHead : curGenGameObjectHeads)
		{
			Object::GameObject* curGenGameObject = curGenGameObjectHead;
			while (curGenGameObject)
			{
				//Update components
				if (curGenGameObject->_typeSqueueComponentsHeadMap.count(targetComponentType))
				{
					for (auto iterator = curGenGameObject->_typeSqueueComponentsHeadMap[targetComponentType]->GetIterator(); iterator.IsValid(); iterator++)
					{
						auto component = static_cast<Component::Component*>(iterator.Node());

						component->Update();

						targetComponents.emplace_back(component);
					}
				}

				//Add next gen GameObject
				if (curGenGameObject->HaveChild())
				{
					auto childHead = curGenGameObject->Child();

					nextGenGameObjectHeads.emplace_back(childHead);
				}

				curGenGameObject = curGenGameObject->Brother();
			}

		}

		curGenGameObjectHeads.clear();
		std::swap(nextGenGameObjectHeads, curGenGameObjectHeads);
	}

	Utils::Log::Message("Core::Thread::LogicThread finish iterate " + std::to_string(static_cast<int>(targetComponentType)) + " by static BFS with record.");
	return targetComponents;
}

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
	//Core::Object::GameObject* go0 = new Core::Object::GameObject("go0");
	//Core::Instance::rootObject.AddChild(go0);

	//Core::Object::GameObject* go1 = new Core::Object::GameObject("go1");
	//Core::Instance::rootObject.AddChild(go1);

	//Core::Object::GameObject* go2 = new Core::Object::GameObject("go2");
	//Core::Instance::rootObject.AddChild(go2);

	//Core::Object::GameObject* go00 = new Core::Object::GameObject("go00");
	//go0->AddChild(go00);

	//Core::Object::GameObject* go01 = new Core::Object::GameObject("go01");
	//go0->AddChild(go01);

	//Core::Object::GameObject* go10 = new Core::Object::GameObject("go10");
	//go1->AddChild(go10);

	//Core::Object::GameObject* go11 = new Core::Object::GameObject("go11");
	//go1->AddChild(go11);

	//Core::Object::GameObject* go20 = new Core::Object::GameObject("go20");
	//go2->AddChild(go20);

	//{
	//	go0->AddComponent(new Test::TestCppBehaviour("go0:1"));
	//	go0->AddComponent(new Test::TestCppBehaviour("go0:2"));
	//	go0->AddComponent(new Test::TestCppBehaviour("go0:3"));
	//	go0->AddComponent(new Test::TestCppBehaviour("go0:4"));
	//	go0->AddComponent(new Test::TestCppBehaviour("go0:5"));
	//	go0->AddComponent(new Test::TestCppBehaviour("go0:6"));
	//	go0->AddComponent(new Test::TestCppBehaviour("go0:7"));
	//	go0->AddComponent(new Test::TestCppBehaviour("go0:8"));
	//	go0->AddComponent(new Test::TestCppBehaviour("go0:9"));
	//	auto f1 = go0->GetComponent<Test::TestCppBehaviour>();
	//	auto r1 = go0->RemoveComponent<Test::TestCppBehaviour>();
	//	auto f2 = go0->GetComponent("Test::TestCppBehaviour");
	//	go0->RemoveComponent(f2);
	//	auto f3 = go0->GetComponents<Test::TestCppBehaviour>();
	//	go0->RemoveComponents(std::vector<Component::Component*>({ f3[0], f3[1] }));
	//	auto f4 = go0->GetComponents("Test::TestCppBehaviour");
	//	auto r2 = go0->RemoveComponents<Test::TestCppBehaviour>();
	//}
	//{
	//	go0->AddComponent(new Test::TestCppBehaviour());
	//	go0->AddComponent(new Test::TestCppBehaviour());
	//	go0->AddComponent(new Test::TestCppBehaviour());
	//	for (const auto& foundTransform : Core::Instance::rootObject.Child()->GetComponents("Test::TestCppBehaviour"))
	//	{
	//		Core::Manager::ObjectFactory::Destroy(foundTransform);
	//	}
	//}
	//{
	//	go0->AddComponent(new Test::TestCppBehaviour());
	//	go00->AddComponent(new Test::TestCppBehaviour());
	//	go01->AddComponent(new Test::TestCppBehaviour());
	//	Core::Manager::ObjectFactory::Destroy(go0);
	//}
	//{
	//	go0->AddComponent(new Test::TestCppBehaviour("go0:1"));
	//	go0->AddComponent(new Test::TestCppBehaviour("go0:2"));
	//	go0->AddComponent(new Test::TestCppBehaviour("go0:3"));
	//	go0->AddComponent(new Test::TestCppBehaviour("go0:4"));
	//	go0->AddComponent(new Test::TestCppBehaviour("go0:5"));
	//	go0->AddComponent(new Test::TestCppBehaviour("go0:6"));
	//	go0->AddComponent(new Test::TestCppBehaviour("go0:7"));
	//	go0->AddComponent(new Test::TestCppBehaviour("go0:8"));
	//	go0->AddComponent(new Test::TestCppBehaviour("go0:9"));

	//	go1->AddComponent(new Test::TestCppBehaviour("go1:1"));
	//	go2->AddComponent(new Test::TestCppBehaviour("go2:1"));
	//	go00->AddComponent(new Test::TestCppBehaviour("go00:1"));
	//	go01->AddComponent(new Test::TestCppBehaviour("go01:1"));
	//	go10->AddComponent(new Test::TestCppBehaviour("go10:1"));
	//	go10->AddComponent(new Test::TestCppBehaviour("go10:2", go1));
	//	go11->AddComponent(new Test::TestCppBehaviour("go11:1"));
	//	go20->AddComponent(new Test::TestCppBehaviour("go20:1", go0));
	//	go20->AddComponent(new Test::TestCppBehaviour("go20:2"));
	//}

	Core::Object::GameObject* cameraGo = new Core::Object::GameObject("Camera");
	cameraGo->AddComponent(new Component::Camera::OrthographicCamera());
	Core::Instance::rootObject.AddChild(cameraGo);

	Core::Object::GameObject* meshRendererGo = new Core::Object::GameObject("MeshRenderer");
	meshRendererGo->AddComponent(new Component::Renderer::MeshRenderer());
	meshRendererGo->AddComponent(new Test::RenderTestBehaviour());
	Core::Instance::rootObject.AddChild(meshRendererGo);


	while (!_stopped)
	{	
		Utils::Log::Message("----------------------------------------------------");
		IterateByDynamicBFS(Component::Component::ComponentType::BEHAVIOUR);

		auto cameras = IterateByStaticBFSWithRecord(Component::Component::ComponentType::CAMERA);
		Utils::Log::Message("Get " + std::to_string(cameras.size()) + " camera.");
		auto renderers = IterateByStaticBFSWithRecord(Component::Component::ComponentType::RENDERER);
		Utils::Log::Message("Get " + std::to_string(renderers.size()) + " renderer.");

		Utils::Log::Message("Core::Thread::LogicThread awake render start.");
		Graphic::Core::Instance::RenderStartCondition().Awake();

		Graphic::Core::Instance::RenderEndCondition().Wait();
		Utils::Log::Message("Core::Thread::LogicThread finish wait render finish.");
		Utils::Log::Message("----------------------------------------------------\n");
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