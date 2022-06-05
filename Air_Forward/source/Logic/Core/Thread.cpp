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
#include "Logic/Component/Camera/PerspectiveCamera.h"
#include "Logic/Component/Renderer/MeshRenderer.h"
#include "Logic/Component/Renderer/BackgroundRenderer.h"
#include "Logic/Component/Renderer/TransparentRenderer.h"
#include "Test/RenderTestBehaviour.h"
#include "Test/BackgroundRendererBehaviour.h"
#include "Test/TransparentRendererBehaviour.h"
#include "Logic/Component/Light/DirectionalLight.h"
#include <algorithm>
#include "Logic/Component/Light/PointLight.h"
#include "Logic/Component/Light/SkyBox.h"
#include "Test/MirrorShaderBehaviour.h"
#include "Test/GlassShaderBehaviour.h"
#include "Test/PointLightMoveBehaviour.h"
#include "Test/CameraMoveBehaviour.h"

Logic::Core::Thread::LogicThread Logic::Core::Thread::_logicThread = Logic::Core::Thread::LogicThread();

void Logic::Core::Thread::LogicThread::IterateByDynamicBfs(Logic::Component::Component::ComponentType targetComponentType)
{
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

	Utils::Log::Message("Core::Thread::LogicThread iterate " + std::to_string(static_cast<int>(targetComponentType)) + " by dynamic BFS.");

}

void Logic::Core::Thread::LogicThread::IterateByStaticBfs(Logic::Component::Component::ComponentType targetComponentType)
{

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

	Utils::Log::Message("Core::Thread::LogicThread iterate " + std::to_string(static_cast<int>(targetComponentType)) + " by static BFS.");
}

void Logic::Core::Thread::LogicThread::IterateByStaticBfs(Logic::Component::Component::ComponentType targetComponentType, std::vector<Component::Component*>& targetComponents)
{

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

	Utils::Log::Message("Core::Thread::LogicThread iterate " + std::to_string(static_cast<int>(targetComponentType)) + " by static BFS with record.");
}

void Logic::Core::Thread::LogicThread::IterateByStaticBfs(std::vector<Logic::Component::Component::ComponentType> targetComponentTypes)
{
	std::string targetComponentTypeString = "";
	for (const auto& type : targetComponentTypes)
	{
		targetComponentTypeString += std::to_string(static_cast<int>(type)) + " ";
	}

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

	Utils::Log::Message("Core::Thread::LogicThread iterate " + targetComponentTypeString + "by static BFS with record.");
}

void Logic::Core::Thread::LogicThread::IterateByStaticBfs(std::vector<Logic::Component::Component::ComponentType> targetComponentTypes, std::vector<std::vector<Component::Component*>>& targetComponents)
{
	std::string targetComponentTypeString = "";
	for (const auto& type : targetComponentTypes)
	{
		targetComponentTypeString += std::to_string(static_cast<int>(type)) + " ";
	}

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

	Utils::Log::Message("Core::Thread::LogicThread iterate " + targetComponentTypeString + "by static BFS with record.");
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
	//Camera
	Logic::Object::GameObject* cameraGo = new Logic::Object::GameObject("Camera");
	Core::Instance::rootObject.AddChild(cameraGo);
	cameraGo->AddComponent(new Component::Camera::PerspectiveCamera());
	cameraGo->AddComponent(new Test::CameraMoveBehaviour());

	//Renderers
	Logic::Object::GameObject* renderers = new Logic::Object::GameObject("Renderers");
	Core::Instance::rootObject.AddChild(renderers);

	Logic::Object::GameObject* backgroundRendererGo = new Logic::Object::GameObject("BackgroundRenderer");
	renderers->AddChild(backgroundRendererGo);
	backgroundRendererGo->AddComponent(new Component::Renderer::BackgroundRenderer());
	backgroundRendererGo->AddComponent(new Test::BackgroundRendererBehaviour());

	Logic::Object::GameObject* meshRendererGo = new Logic::Object::GameObject("meshRenderer");
	renderers->AddChild(meshRendererGo);
	meshRendererGo->AddComponent(new Component::Renderer::MeshRenderer());
	meshRendererGo->AddComponent(new Test::RenderTestBehaviour());

	Logic::Object::GameObject* glassMeshRendererGo = new Logic::Object::GameObject("GlassMeshRenderer");
	renderers->AddChild(glassMeshRendererGo);
	glassMeshRendererGo->AddComponent(new Component::Renderer::MeshRenderer());
	glassMeshRendererGo->AddComponent(new Test::GlassShaderBehaviour());
	glassMeshRendererGo->transform.SetTranslation(glm::vec3(3, 0, 0));

	Logic::Object::GameObject* mirrorMeshRendererGo = new Logic::Object::GameObject("MirrorMeshRenderer");
	renderers->AddChild(mirrorMeshRendererGo);
	mirrorMeshRendererGo->AddComponent(new Component::Renderer::MeshRenderer());
	mirrorMeshRendererGo->AddComponent(new Test::MirrorShaderBehaviour());
	mirrorMeshRendererGo->transform.SetTranslation(glm::vec3(-3, 0, 0));

	Logic::Object::GameObject* meshRendererCulledGo = new Logic::Object::GameObject("MeshRendererCulled");
	renderers->AddChild(meshRendererCulledGo);
	meshRendererCulledGo->AddComponent(new Component::Renderer::MeshRenderer());
	meshRendererCulledGo->AddComponent(new Test::RenderTestBehaviour());
	meshRendererCulledGo->transform.SetTranslation(glm::vec3(2000, 2000, 2000));

	//Lights
	Logic::Object::GameObject* transparentRenderers = new Logic::Object::GameObject("TransparentRenderers");
	renderers->AddChild(transparentRenderers);
	{
		Logic::Object::GameObject* transparentRendererGo = new Logic::Object::GameObject("TransparentRenderer1");
		transparentRenderers->AddChild(transparentRendererGo);
		transparentRendererGo->AddComponent(new Component::Renderer::TransparentRenderer());
		transparentRendererGo->AddComponent(new Test::TransparentRendererBehaviour());
		transparentRendererGo->transform.SetScale(glm::vec3(2, 2, 2));
		transparentRendererGo->transform.SetTranslation(glm::vec3(2, 2, 0));
		transparentRendererGo->transform.SetEulerRotation(glm::vec3(90, 0, 0));
	}
	{
		Logic::Object::GameObject* transparentRendererGo = new Logic::Object::GameObject("TransparentRenderer2");
		transparentRenderers->AddChild(transparentRendererGo);
		transparentRendererGo->AddComponent(new Component::Renderer::TransparentRenderer());
		transparentRendererGo->AddComponent(new Test::TransparentRendererBehaviour());
		transparentRendererGo->transform.SetScale(glm::vec3(2, 2, 2));
		transparentRendererGo->transform.SetTranslation(glm::vec3(-2, 2, 0));
		transparentRendererGo->transform.SetEulerRotation(glm::vec3(90, 0, 0));
	}
	{
		Logic::Object::GameObject* transparentRendererGo = new Logic::Object::GameObject("TransparentRenderer3");
		transparentRenderers->AddChild(transparentRendererGo);
		transparentRendererGo->AddComponent(new Component::Renderer::TransparentRenderer());
		transparentRendererGo->AddComponent(new Test::TransparentRendererBehaviour());
		transparentRendererGo->transform.SetScale(glm::vec3(2, 2, 2));
		transparentRendererGo->transform.SetTranslation(glm::vec3(2, 0, 2));
	}
	{
		Logic::Object::GameObject* transparentRendererGo = new Logic::Object::GameObject("TransparentRenderer4");
		transparentRenderers->AddChild(transparentRendererGo);
		transparentRendererGo->AddComponent(new Component::Renderer::TransparentRenderer());
		transparentRendererGo->AddComponent(new Test::TransparentRendererBehaviour());
		transparentRendererGo->transform.SetScale(glm::vec3(2, 2, 2));
		transparentRendererGo->transform.SetTranslation(glm::vec3(-2, 0, 2));
	}
	{
		Logic::Object::GameObject* transparentRendererGo = new Logic::Object::GameObject("TransparentRenderer5");
		transparentRenderers->AddChild(transparentRendererGo);
		transparentRendererGo->AddComponent(new Component::Renderer::TransparentRenderer());
		transparentRendererGo->AddComponent(new Test::TransparentRendererBehaviour());
		transparentRendererGo->transform.SetScale(glm::vec3(2, 2, 2));
		transparentRendererGo->transform.SetTranslation(glm::vec3(4, 0, 0));
		transparentRendererGo->transform.SetEulerRotation(glm::vec3(90, 90, 90));
	}


	//Lights
	Logic::Object::GameObject* lights = new Logic::Object::GameObject("Lights");
	Core::Instance::rootObject.AddChild(lights);

	Logic::Object::GameObject* directionalLightGo = new Logic::Object::GameObject("DirectionalLight");
	lights->AddChild(directionalLightGo);
	directionalLightGo->transform.SetEulerRotation(glm::vec3(-30, 70, 0));
	auto directionalLight = new Component::Light::DirectionalLight();
	directionalLight->color = { 1, 239.0 / 255, 213.0 / 255, 1 };
	directionalLight->intensity = 0.4;
	directionalLightGo->AddComponent(directionalLight);

	Logic::Object::GameObject* skyBoxGo = new Logic::Object::GameObject("SkyBox");
	lights->AddChild(skyBoxGo);
	auto skyBox = new Component::Light::SkyBox();
	skyBox->color = { 1, 1, 1, 1 };
	skyBox->intensity = 0.4f;
	skyBoxGo->AddComponent(skyBox);

	float sr6 = std::pow(6.0, 0.5);
	float sr2 = std::pow(2.0, 0.5);

	Logic::Object::GameObject* nearPointLights = new Logic::Object::GameObject("NearPointLights");
	lights->AddChild(nearPointLights);
	nearPointLights->AddComponent(new Test::PointLightMoveBehaviour(60));
	{
		Logic::Object::GameObject* pointLightGo = new Logic::Object::GameObject("NearPointLight 1");
		nearPointLights->AddChild(pointLightGo);
		pointLightGo->transform.SetTranslation({ 0, 0, 2 });
		auto pointLight = new Component::Light::PointLight();
		pointLight->color = { 1, 1, 0, 1 };
		pointLight->minRange = 1;
		pointLight->maxRange = 4;
		pointLightGo->AddComponent(pointLight);
	}
	{
		Logic::Object::GameObject* pointLightGo = new Logic::Object::GameObject("NearPointLight 2");
		nearPointLights->AddChild(pointLightGo);
		pointLightGo->transform.SetTranslation({ -sr6 * 2 / 3, -sr2 * 2 / 3, - 2.0 / 3 });
		auto pointLight = new Component::Light::PointLight();
		pointLight->color = { 1, 0, 0, 1 };
		pointLight->minRange = 1;
		pointLight->maxRange = 4;
		pointLightGo->AddComponent(pointLight);
	}
	{
		Logic::Object::GameObject* pointLightGo = new Logic::Object::GameObject("NearPointLight 3");
		nearPointLights->AddChild(pointLightGo);
		pointLightGo->transform.SetTranslation({ sr6 * 2 / 3, -sr2 * 2 / 3, - 2.0 / 3 });
		auto pointLight = new Component::Light::PointLight();
		pointLight->color = { 0, 1, 0, 1 };
		pointLight->minRange = 1;
		pointLight->maxRange = 4;
		pointLightGo->AddComponent(pointLight);
	}
	{
		Logic::Object::GameObject* pointLightGo = new Logic::Object::GameObject("NearPointLight 4");
		nearPointLights->AddChild(pointLightGo);
		pointLightGo->transform.SetTranslation({ 0, sr2 * 2 * 2 / 3, - 2.0 / 3 });
		auto pointLight = new Component::Light::PointLight();
		pointLight->color = { 0, 0, 1, 1 };
		pointLight->minRange = 1;
		pointLight->maxRange = 4;
		pointLightGo->AddComponent(pointLight);
	}

	//Logic::Object::GameObject* farPointLights = new Logic::Object::GameObject("FarPointLights");
	//lights->AddChild(farPointLights);
	//farPointLights->AddComponent(new Test::PointLightMoveBehaviour(60));
	//{
	//	Logic::Object::GameObject* pointLightGo = new Logic::Object::GameObject("FarPointLight 1");
	//	farPointLights->AddChild(pointLightGo);
	//	pointLightGo->transform.SetTranslation({ 0, 0, -4 });
	//	auto pointLight = new Component::Light::PointLight();
	//	pointLight->color = { 0, 1, 1, 1 };
	//	pointLight->minRange = 1;
	//	pointLight->maxRange = 6;
	//	pointLightGo->AddComponent(pointLight);
	//}
	//{
	//	Logic::Object::GameObject* pointLightGo = new Logic::Object::GameObject("FarPointLight 2");
	//	farPointLights->AddChild(pointLightGo);
	//	pointLightGo->transform.SetTranslation({ -sr6 * 4 / 3, sr2 * 4 / 3, 4.0 / 3 });
	//	auto pointLight = new Component::Light::PointLight();
	//	pointLight->color = { 1, 0, 1, 1 };
	//	pointLight->minRange = 1;
	//	pointLight->maxRange = 6;
	//	pointLightGo->AddComponent(pointLight);
	//}
	//{
	//	Logic::Object::GameObject* pointLightGo = new Logic::Object::GameObject("FarPointLight 3");
	//	farPointLights->AddChild(pointLightGo);
	//	pointLightGo->transform.SetTranslation({ sr6 * 4 / 3, sr2 * 4 / 3, 4.0 / 3 });
	//	auto pointLight = new Component::Light::PointLight();
	//	pointLight->color = { 1, 1, 1, 1 };
	//	pointLight->minRange = 1;
	//	pointLight->maxRange = 6;
	//	pointLightGo->AddComponent(pointLight);
	//}
	//{
	//	Logic::Object::GameObject* pointLightGo = new Logic::Object::GameObject("FarPointLight 4");
	//	farPointLights->AddChild(pointLightGo);
	//	pointLightGo->transform.SetTranslation({ 0, -sr2 * 2 * 4 / 3, 4.0 / 3 });
	//	auto pointLight = new Component::Light::PointLight();
	//	pointLight->color = { 1, 1, 0, 1 };
	//	pointLight->minRange = 1;
	//	pointLight->maxRange = 6;
	//	pointLightGo->AddComponent(pointLight);
	//}

	while (!_stopped)
	{	
		Utils::Log::Message("----------------------------------------------------");
		
		Instance::time.Refresh();

		IterateByDynamicBfs(Component::Component::ComponentType::BEHAVIOUR);
		auto cameras = std::vector<Logic::Component::Component*>();

		auto targetComponents = std::vector<std::vector<Logic::Component::Component*>>();
		IterateByStaticBfs({ Component::Component::ComponentType::LIGHT, Component::Component::ComponentType::CAMERA, Component::Component::ComponentType::RENDERER }, targetComponents);

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