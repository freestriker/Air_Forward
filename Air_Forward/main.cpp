#include <rttr/registration>
#include <iostream>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <FreeImage/FreeImage.h>
#include "Core/Object/GameObject.h"
#include "Core/Object/Object.h"
#include "core/LoadThread.h"
#include "Graphic/Core/Thread.h"
#include "Core/Component/Transform/Transform.h"
#include "utils/Log.h"
int main()
{
	Core::Object::GameObject* go0 = new Core::Object::GameObject("go0");
	Core::Object::GameObject::RootObject().AddChild(go0);

	Core::Object::GameObject* go1 = new Core::Object::GameObject("go1");
	Core::Object::GameObject::RootObject().AddChild(go1);

	Core::Object::GameObject* go2 = new Core::Object::GameObject("go2");
	Core::Object::GameObject::RootObject().AddChild(go2);

	Core::Object::GameObject* go00 = new Core::Object::GameObject("go00");
	go0->AddChild(go00);

	Core::Object::GameObject* go01 = new Core::Object::GameObject("go01");
	go0->AddChild(go01);

	Core::Component::Transform::Transform* testTransform = new Core::Component::Transform::Transform();
	go0->AddComponent(testTransform);

	auto foundTransforms = go0->GetComponents("Core::Component::Transform::Transform");

	Graphic::Core::Thread::Init();
	LoadThread::instance->Init();

	Graphic::Core::Thread::Start();
	Graphic::Core::Thread::WaitForStartFinish();

	LoadThread::instance->Start();

	Graphic::Core::Thread::StartRender();

	std::this_thread::sleep_for(std::chrono::seconds(20));

	LoadThread::instance->End();
	Graphic::Core::Thread::End();
}