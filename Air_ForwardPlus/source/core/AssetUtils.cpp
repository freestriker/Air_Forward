#include "core/AssetUtils.h"
#include <Graphic/CommandBuffer.h>
#include <core/LoadThread.h>

IAsset::IAsset()
	: _assetInstance(nullptr)
{
}

IAsset::~IAsset()
{
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
	, _readyToUse(false)
{
}

IAssetInstance::~IAssetInstance()
{

}

AssetManager::AssetManager()
	: _warps()
	, _mutex()
{
}

AssetManager::~AssetManager()
{
	std::unique_lock<std::mutex> lock(_mutex);
	for (const auto& pair : _warps)
	{
		for (const auto& asset : pair.second.assets)
		{
			delete asset;
		}
		delete pair.second.assetInstance;
	}
	_warps.clear();
}

void AssetManager::Collect()
{
	std::unique_lock<std::mutex> lock(_mutex);
	std::vector<std::string> collecteds;
	for (auto it = _warps.cbegin(); it != _warps.cend(); )
	{
		if (it->second.refCount == 0)
		{
			collecteds.push_back(it->second.assetInstance->path);
			for (const auto& asset : it->second.assets)
			{
				delete asset;
			}
			delete it->second.assetInstance;
			it = _warps.erase(it++);
		}
		else
		{
			++it;
		}
	}
	for (const auto& collected : collecteds)
	{
		Debug::Message("AssetManager collect " + collected + " .");
	}
}

void AssetManager::_AddInstance(std::string path, IAssetInstance* assetInstance)
{
	_warps.emplace(path, _AssetInstanceWarp{ 0, {}, assetInstance });
}

IAssetInstance* AssetManager::_AcquireInstance(std::string path, IAsset* newAsset)
{
	_warps[path].refCount++;
	_warps[path].assets.emplace(newAsset);
	return _warps[path].assetInstance;
}

bool AssetManager::_ContainsInstance(std::string path)
{
	return _warps.count(path);
}

void AssetManager::_ReleaseInstance(IAssetInstance* assetInstance, IAsset* newAsset)
{
	if (_warps[assetInstance->path].assets.count(newAsset))
	{
		--_warps[assetInstance->path].refCount;
		_warps[assetInstance->path].assets.erase(newAsset);
	}
}
