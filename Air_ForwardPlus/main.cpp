#include <rttr/registration>
#include <iostream>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <FreeImage/FreeImage.h>
#include "core/Object.h"
#include <core/GameObject.h>
#include <core/ObjectFactory.h>
#include <core/Global.h>
#include "core/LoadThread.h"
#include "Graphic/GraphicThread.h"
#include "utils/Log.h"
static void f() { std::cout << "Hello World" << std::endl; }
using namespace rttr;
RTTR_REGISTRATION
{
    using namespace rttr;
    registration::method("f", &f);
}
int main()
{

    //type::invoke("f", {});

	GameObject* go0 = ObjectFactory::InstantiateGameObject();
	go0->name = "go0";
	global.rootGameObject->AddChild(go0);

	GameObject* go1 = ObjectFactory::InstantiateGameObject();
	go1->name = "go1";
	global.rootGameObject->AddChild(go1);

	GameObject* go2 = ObjectFactory::InstantiateGameObject();
	go2->name = "go2";
	global.rootGameObject->AddChild(go2);

	GameObject* go00 = ObjectFactory::InstantiateGameObject();
	go00->name = "go00";
	go0->AddChild(go00);

	GameObject* go01 = ObjectFactory::InstantiateGameObject();
	go01->name = "go01";
	go0->AddChild(go01);

	Transform* testTransform = ObjectFactory::InstantiateComponent<Transform>("Transform", {});
	go0->AddComponent(testTransform);

	Graphic::GraphicThread::instance->Init();
	LoadThread::instance->Init();

	Graphic::GraphicThread::instance->Start();
	Graphic::GraphicThread::instance->WaitForStartFinish();

	LoadThread::instance->Start();

	Graphic::GraphicThread::instance->StartRender();

	std::this_thread::sleep_for(std::chrono::seconds(20));

	LoadThread::instance->End();
	Graphic::GraphicThread::instance->End();
}