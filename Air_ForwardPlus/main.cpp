#include <rttr/registration>
#include <iostream>
#include <glm/glm.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <FreeImage.h>
#include "core/Object.h"
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
    glm::vec3 v = glm::vec3(1);
    LoadModel("C:/Users/FREEstriker/Desktop/sphere.obj");
	LoadTexture("C:/Users/FREEstriker/Desktop/wall.png");
	Object* o = new Object();
	std::cout << o->Type().name() << std::endl;
	std::cout << o->ToString() << std::endl;
}
// outputs: "Hello World"