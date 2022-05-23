#include <rttr/registration>
#include <iostream>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <FreeImage/FreeImage.h>
#include "Core/Object/GameObject.h"
#include "Core/Object/Object.h"
#include "IO/Core/Thread.h"
#include "Graphic/Core/Thread.h"
#include "Core/Component/Transform/Transform.h"
#include "utils/Log.h"
#include "Core/Thread.h"
#include <Core/Instance.h>
int main()
{
	Graphic::Core::Thread::Init();
	Graphic::Core::Thread::Start();
	Graphic::Core::Thread::WaitForStartFinish();

	IO::Core::Thread::Init();
	IO::Core::Thread::Start();
	IO::Core::Thread::WaitForStartFinish();

	Graphic::Core::Thread::StartRender();

	Core::Thread::Init();
	Core::Thread::Start();
	Core::Thread::WaitForStartFinish();

	Core::Instance::WaitExit();

	Core::Thread::End();
	IO::Core::Thread::End();
	Graphic::Core::Thread::End();
}