#include "core/AssetUtils.h"
#include <Graphic/CommandBuffer.h>
#include <core/LoadThread.h>

IAsset::IAsset(IAssetInstance* assetInstance)
	: _assetInstance(assetInstance)
{
}
IAsset::IAsset(const IAsset& source)
{
	std::unique_lock<std::mutex> lock(source._assetInstance->assetManager->mutex);
	_assetInstance = source._assetInstance->assetManager->GetInstance(source._assetInstance->path);
}

IAsset::~IAsset()
{
	std::unique_lock<std::mutex> lock(_assetInstance->assetManager->mutex);
	_assetInstance->assetManager->RecycleInstance(_assetInstance->path);

	_assetInstance = nullptr;
}

void IAssetInstance::_Wait()
{
	while (!this->_readyToUse)
	{
		std::this_thread::yield();
	}
}

IAssetInstance::IAssetInstance(std::string path)
	: path(path)
	, assetManager(LoadThread::instance->assetManager.get())
	, _readyToUse(false)
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
