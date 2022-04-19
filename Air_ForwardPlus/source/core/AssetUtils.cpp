#include "core/AssetUtils.h"

IAsset::IAsset(IAssetInstance* assetInstance)
	: assetInstance(assetInstance)
{
}

IAsset::~IAsset()
{
	std::unique_lock<std::mutex> lock(assetInstance->assetManager->mutex);
	assetInstance->assetManager->RecycleInstance(assetInstance->path);

	assetInstance = nullptr;
}


IAssetInstance::IAssetInstance(std::string path, AssetManager* assetManager)
	: path(path)
	, assetManager(assetManager)
{
}

IAssetInstance::~IAssetInstance()
{

}

AssetManager::AssetManager()
	: _warps()
	, mutex()
{
}

AssetManager::~AssetManager()
{
	std::unique_lock<std::mutex> lock(mutex);
	for (const auto& pair : _warps)
	{
		delete pair.second.assetInstance;
	}
	_warps.clear();
}

void AssetManager::AddInstance(std::string path, IAssetInstance* assetInstance)
{
	_warps.emplace(path, AssetInstanceWarp{ 0, assetInstance });
}

IAssetInstance* AssetManager::GetInstance(std::string path)
{
	_warps[path].refCount++;
	return _warps[path].assetInstance;
}

bool AssetManager::ContainsInstance(std::string path)
{
	return _warps.count(path);
}

void AssetManager::RecycleInstance(std::string path)
{
	--_warps[path].refCount;
}
