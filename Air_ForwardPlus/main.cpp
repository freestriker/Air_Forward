#include <rttr/registration>
#include <iostream>
#include <glm/glm.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <FreeImage/FreeImage.h>
#include "core/Object.h"
#include "utils/ChildBrotherTree.h"
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
//typedef ChildBrotherTree<int> Node;
class Node : public ChildBrotherTree<int>
{
public:
	Node(int* data) : ChildBrotherTree<int>(data)
	{

	}
	void OnAdd(int* parent)
	{
		std::cout << *this->object << std::endl;
	}
	void OnRemove()
	{
	
	}
};
Node* root = new Node(nullptr);
int data[5] = { 0, 1, 2, 3, 4 };
int main()
{
    type::invoke("f", {});
    glm::vec3 v = glm::vec3(1);
 //   LoadModel("C:/Users/FREEstriker/Desktop/sphere.obj");
	//LoadTexture("C:/Users/FREEstriker/Desktop/wall.png");
	Object* o = new Object();
	std::cout << o->TypeName() << std::endl;
	std::cout << o->ToString() << std::endl;

	Node* n1 = new Node(data);
	Node* n2 = new Node(data + 1);
	Node* n3 = new Node(data + 2);
	Node* n11 = new Node(data + 3);
	Node* n21 = new Node(data + 4);

	root->AddChild(n1);
	root->AddChild(n2);
	n2->AddBrother(n3);
	n1->AddChild(n11);
	n2->AddChild(n21);

}
// outputs: "Hello World"