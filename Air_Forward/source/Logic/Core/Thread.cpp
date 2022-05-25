#include "Logic/Core/Thread.h"
#include <Logic/Object/GameObject.h>
#include "Logic/Core/Instance.h"
#include "Utils/Log.h"
#include "Logic/Manager/ObjectFactory.h"
#include <list>
#include "Test/TestCppBehaviour.h"
#include "Graphic/Core/Instance.h"
#include <string>
#include "Logic/Component/Camera/OrthographicCamera.h"
#include "Logic/Component/Renderer/MeshRenderer.h"
#include "Test/RenderTestBehaviour.h"
#include "Logic/Component/Light/DirectionalLight.h"
#include "Logic/Component/Light/PointLight.h"

Logic::Core::Thread::LogicThread Logic::Core::Thread::_logicThread = Logic::Core::Thread::LogicThread();

void Logic::Core::Thread::LogicThread::IterateByDynamicBfs(Logic::Component::Component::ComponentType targetComponentType)
{
	Utils::Log::Message("Core::Thread::LogicThread start iterate " + std::to_string(static_cast<int>(targetComponentType)) + " by dynamic BFS.");
	std::list< Logic::Object::GameObject*> curGenGameObjectHeads = std::list<Logic::Object::GameObject*>();
	std::list< Logic::Object::GameObject*> nextGenGameObjectHeads = std::list<Logic::Object::GameObject*>();
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

			std::vector<Logic::Object::GameObject*> curGenGameObjects = std::vector<Logic::Object::GameObject*>();
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

void Logic::Core::Thread::LogicThread::IterateByStaticBfs(Logic::Component::Component::ComponentType targetComponentType)
{
	Utils::Log::Message("Core::Thread::LogicThread start iterate " + std::to_string(static_cast<int>(targetComponentType)) + " by static BFS.");

	std::list< Logic::Object::GameObject*> curGenGameObjectHeads = std::list<Logic::Object::GameObject*>();
	std::list< Logic::Object::GameObject*> nextGenGameObjectHeads = std::list<Logic::Object::GameObject*>();

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

void Logic::Core::Thread::LogicThread::IterateByStaticBfs(Logic::Component::Component::ComponentType targetComponentType, std::vector<Component::Component*>& targetComponents)
{
	Utils::Log::Message("Core::Thread::LogicThread start iterate " + std::to_string(static_cast<int>(targetComponentType)) + " by static BFS with record.");

	std::list< Logic::Object::GameObject*> curGenGameObjectHeads = std::list<Logic::Object::GameObject*>();
	std::list< Logic::Object::GameObject*> nextGenGameObjectHeads = std::list<Logic::Object::GameObject*>();

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
}

void Logic::Core::Thread::LogicThread::IterateByStaticBfs(std::vector<Logic::Component::Component::ComponentType> targetComponentTypes)
{
	std::string targetComponentTypeString = "";
	for (const auto& type : targetComponentTypes)
	{
		targetComponentTypeString += std::to_string(static_cast<int>(type)) + " ";
	}
	Utils::Log::Message("Core::Thread::LogicThread start iterate " + targetComponentTypeString + "by static BFS with record.");

	std::list< Logic::Object::GameObject*> curGenGameObjectHeads = std::list<Logic::Object::GameObject*>();
	std::list< Logic::Object::GameObject*> nextGenGameObjectHeads = std::list<Logic::Object::GameObject*>();

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
				for (uint32_t i = 0; i < targetComponentTypes.size(); i++)
				{
					if (curGenGameObject->_typeSqueueComponentsHeadMap.count(targetComponentTypes[i]))
					{
						for (auto iterator = curGenGameObject->_typeSqueueComponentsHeadMap[targetComponentTypes[i]]->GetIterator(); iterator.IsValid(); iterator++)
						{
							auto component = static_cast<Component::Component*>(iterator.Node());

							component->Update();

						}
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

	Utils::Log::Message("Core::Thread::LogicThread finish iterate " + targetComponentTypeString + "by static BFS with record.");
}

void Logic::Core::Thread::LogicThread::IterateByStaticBfs(std::vector<Logic::Component::Component::ComponentType> targetComponentTypes, std::vector<std::vector<Component::Component*>>& targetComponents)
{
	std::string targetComponentTypeString = "";
	for (const auto& type : targetComponentTypes)
	{
		targetComponentTypeString += std::to_string(static_cast<int>(type)) + " ";
	}
	Utils::Log::Message("Core::Thread::LogicThread start iterate " + targetComponentTypeString + "by static BFS with record.");

	std::list< Logic::Object::GameObject*> curGenGameObjectHeads = std::list<Logic::Object::GameObject*>();
	std::list< Logic::Object::GameObject*> nextGenGameObjectHeads = std::list<Logic::Object::GameObject*>();
	targetComponents.clear();
	targetComponents.resize(targetComponentTypes.size());

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
				for (uint32_t i = 0; i < targetComponentTypes.size(); i++)
				{
					if (curGenGameObject->_typeSqueueComponentsHeadMap.count(targetComponentTypes[i]))
					{
						for (auto iterator = curGenGameObject->_typeSqueueComponentsHeadMap[targetComponentTypes[i]]->GetIterator(); iterator.IsValid(); iterator++)
						{
							auto component = static_cast<Component::Component*>(iterator.Node());

							component->Update();

							targetComponents[i].emplace_back(component);
						}
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

	Utils::Log::Message("Core::Thread::LogicThread finish iterate " + targetComponentTypeString + "by static BFS with record.");
}

Logic::Core::Thread::LogicThread::LogicThread()
	:_stopped(true)
{
}

Logic::Core::Thread::LogicThread::~LogicThread()
{
}

void Logic::Core::Thread::LogicThread::Init()
{
}

void Logic::Core::Thread::LogicThread::OnStart()
{
	_stopped = false;
	Instance::time.Launch();
}

void Logic::Core::Thread::LogicThread::OnThreadStart()
{
}

void Logic::Core::Thread::LogicThread::OnRun()
{
	Logic::Object::GameObject* cameraGo = new Logic::Object::GameObject("Camera");
	Core::Instance::rootObject.AddChild(cameraGo);
	cameraGo->AddComponent(new Component::Camera::OrthographicCamera());
	cameraGo->transform.SetTranslation(glm::vec3(0, 0, 5));

	Logic::Object::GameObject* meshRendererGo1 = new Logic::Object::GameObject("MeshRenderer1");
	Core::Instance::rootObject.AddChild(meshRendererGo1);
	meshRendererGo1->AddComponent(new Component::Renderer::MeshRenderer());
	meshRendererGo1->AddComponent(new Test::RenderTestBehaviour());
	meshRendererGo1->transform.SetTranslation(glm::vec3(0, 0, 0));

	Logic::Object::GameObject* meshRendererGo2 = new Logic::Object::GameObject("MeshRenderer2");
	Core::Instance::rootObject.AddChild(meshRendererGo2);
	meshRendererGo2->AddComponent(new Component::Renderer::MeshRenderer());
	meshRendererGo2->AddComponent(new Test::RenderTestBehaviour());
	meshRendererGo2->transform.SetTranslation(glm::vec3(20, 0, -15));

	Logic::Object::GameObject* meshRendererClipGo = new Logic::Object::GameObject("MeshRendererClip");
	Core::Instance::rootObject.AddChild(meshRendererClipGo);
	meshRendererClipGo->AddComponent(new Component::Renderer::MeshRenderer());
	meshRendererClipGo->AddComponent(new Test::RenderTestBehaviour());
	meshRendererClipGo->transform.SetTranslation(glm::vec3(10000, 0, -30));

	Logic::Object::GameObject* directionalLightGo = new Logic::Object::GameObject("DirectionalLight");
	Core::Instance::rootObject.AddChild(directionalLightGo);
	auto directionalLight = new Component::Light::DirectionalLight();
	directionalLight->color = { 0, 1, 0, 1 };
	directionalLightGo->AddComponent(directionalLight);
	directionalLightGo->transform.SetEulerRotation(glm::vec3(0, 30, 0));

	Logic::Object::GameObject* pointLightGo = new Logic::Object::GameObject("PointLight");
	Core::Instance::rootObject.AddChild(pointLightGo);
	auto pointLight = new Component::Light::PointLight();
	pointLight->color = { 1, 0, 0, 1 };
	pointLight->range = 10;
	pointLightGo->AddComponent(pointLight);
	pointLightGo->transform.SetTranslation(glm::vec3(5, 5, 5));


	while (!_stopped)
	{	
		Utils::Log::Message("----------------------------------------------------");
		
		Instance::time.Refresh();

		IterateByDynamicBfs(Component::Component::ComponentType::BEHAVIOUR);
		auto cameras = std::vector<Logic::Component::Component*>();

		auto targetComponents = std::vector<std::vector<Logic::Component::Component*>>();
		IterateByStaticBfs({ Component::Component::ComponentType::LIGHT, Component::Component::ComponentType::CAMERA, Component::Component::ComponentType::RENDERER }, targetComponents);
		Utils::Log::Message("Get " + std::to_string(targetComponents[0].size()) + " light.");
		Utils::Log::Message("Get " + std::to_string(targetComponents[1].size()) + " camera.");
		Utils::Log::Message("Get " + std::to_string(targetComponents[2].size()) + " renderer.");

		Graphic::Core::Instance::AddLight(targetComponents[0]);
		Graphic::Core::Instance::AddCamera(targetComponents[1]);
		Graphic::Core::Instance::AddRenderer(targetComponents[2]);

		Utils::Log::Message("Core::Thread::LogicThread awake render start.");
		Graphic::Core::Instance::RenderStartCondition().Awake();
		//RenderThread render
		Graphic::Core::Instance::RenderEndCondition().Wait();
		Utils::Log::Message("Core::Thread::LogicThread finish wait render finish.");
		
		Utils::Log::Message("----------------------------------------------------\n");
	}
}

void Logic::Core::Thread::LogicThread::OnEnd()
{
	_stopped = true;
}

Logic::Core::Thread::Thread()
{
}
Logic::Core::Thread::~Thread()
{
}

void Logic::Core::Thread::Init()
{
	_logicThread.Init();
}

void Logic::Core::Thread::Start()
{
	_logicThread.Start();
}

void Logic::Core::Thread::End()
{
	_logicThread.End();
}
void Logic::Core::Thread::WaitForStartFinish()
{
	_logicThread.WaitForStartFinish();
}