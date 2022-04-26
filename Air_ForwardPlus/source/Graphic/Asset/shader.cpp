#include "Graphic/Asset/Shader.h"

Graphic::Asset::Shader::ShaderInstance::ShaderInstance(std::string path)
	: IAssetInstance(path)
{
}

Graphic::Asset::Shader::ShaderInstance::~ShaderInstance()
{
}

void Graphic::Asset::Shader::ShaderInstance::_LoadAssetInstance(Graphic::CommandBuffer* const transferCommandBuffer, Graphic::CommandBuffer* const renderCommandBuffer)
{
}

Graphic::Asset::Shader::Shader(const Shader& source)
	: IAsset(source)
{
}
Graphic::Asset::Shader::Shader(ShaderInstance* assetInstance)
	: IAsset(assetInstance)
{
}

Graphic::Asset::Shader::~Shader()
{
}

std::future<Graphic::Asset::Shader*> Graphic::Asset::Shader::LoadAsync(const char* path)
{
	return _LoadAsync<Graphic::Asset::Shader, Graphic::Asset::Shader::ShaderInstance>(path);
}

Graphic::Asset::Shader* Graphic::Asset::Shader::Load(const char* path)
{
	return _Load<Graphic::Asset::Shader, Graphic::Asset::Shader::ShaderInstance>(path);
}

Graphic::Asset::Shader::ShaderData::ShaderData()
{
}

Graphic::Asset::Shader::ShaderData::~ShaderData()
{
}
