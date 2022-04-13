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
static void f() { std::cout << "Hello World" << std::endl; }
using namespace rttr;
RTTR_REGISTRATION
{
    using namespace rttr;
    registration::method("f", &f);
}
void LoadModel(std::string path)
{
    Assimp::Importer import;
    const aiScene* scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
        return;
    }
    std::string directory = path.substr(0, path.find_last_of('/'));
    std::cout << directory << std::endl;

}
void LoadTexture(std::string path)
{
	FreeImage_Initialise(TRUE);

	const char* filePath = path.c_str();
	FIBITMAP* texture = nullptr;
	FREE_IMAGE_FORMAT fif = FreeImage_GetFileType(filePath);
	if (fif == FREE_IMAGE_FORMAT::FIF_UNKNOWN)
		fif = FreeImage_GetFIFFromFilename(filePath);

	if ((fif != FREE_IMAGE_FORMAT::FIF_UNKNOWN) && FreeImage_FIFSupportsReading(fif))
	{
		texture = FreeImage_Load(fif, filePath);
	}
	else
	{
		throw "Can not read texture " + path + ".";
	}
	if (texture == nullptr)
	{
		throw "Read texture " + path + " failed.";
	}
	else
	{
		std::cout << "Load Texture Success!" << std::endl;
	}
}
int main()
{

    type::invoke("f", {});

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

	LoadThread* loadThread = new LoadThread();
	loadThread->Start();

	Graphic::GraphicThread* graphicThread = new Graphic::GraphicThread();
	graphicThread->Start();

	std::this_thread::sleep_for(std::chrono::seconds(20));

	graphicThread->End();
	loadThread->End();
	delete graphicThread;
	delete loadThread;
}
// outputs: "Hello World"