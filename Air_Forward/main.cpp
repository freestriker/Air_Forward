#include <rttr/registration>
#include <iostream>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <FreeImage/FreeImage.h>
#include "Logic/Object/GameObject.h"
#include "Logic/Object/Object.h"
#include "IO/Core/Thread.h"
#include "Graphic/Core/Thread.h"
#include "Logic/Component/Transform/Transform.h"
#include "utils/Log.h"
#include "Logic/Core/Thread.h"
#include "Logic/Core/Instance.h"
int main()
{
	Graphic::Core::Thread::Init();
	Graphic::Core::Thread::Start();
	Graphic::Core::Thread::WaitForStartFinish();

	IO::Core::Thread::Init();
	IO::Core::Thread::Start();
	IO::Core::Thread::WaitForStartFinish();

	Logic::Core::Thread::Init();
	Logic::Core::Thread::Start();
	Logic::Core::Thread::WaitForStartFinish();

	Logic::Core::Instance::WaitExit();

	Logic::Core::Thread::End();
	IO::Core::Thread::End();
	Graphic::Core::Thread::End();
}