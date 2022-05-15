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
#include "Core/Thread.h"
int main()
{
	Core::Thread::Init();
	Graphic::Core::Thread::Init();
	LoadThread::instance->Init();

	Graphic::Core::Thread::Start();
	Graphic::Core::Thread::WaitForStartFinish();
	LoadThread::instance->Start();
	LoadThread::instance->WaitForStartFinish();
	Core::Thread::Start();
	Core::Thread::WaitForStartFinish();

	Graphic::Core::Thread::StartRender();

	std::this_thread::sleep_for(std::chrono::seconds(20));

	Core::Thread::End();
	LoadThread::instance->End();
	Graphic::Core::Thread::End();
}