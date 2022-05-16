#include "IO/Manager/AssetManager.h"
#include "IO/Asset/AssetBase.h"
#include "Utils/Log.h"
#include <vector>
#include <string>
IO::Manager::AssetManager::AssetManager()
	: _warps()
	, _managerMutex()
{
}

IO::Manager::AssetManager::~AssetManager()
{
	std::unique_lock<std::mutex> lock(_managerMutex);
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

void IO::Manager::AssetManager::Collect()
{
	std::unique_lock<std::mutex> lock(_managerMutex);
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
		Utils::Log::Message("AssetManager collect " + collected + " .");
	}
}

void IO::Manager::AssetManager::_AddInstance(std::string path, IO::Asset::IAssetInstance* assetInstance)
{
	_warps.emplace(path, _AssetInstanceWarp{ 0, {}, assetInstance });
}

IO::Asset::IAssetInstance* IO::Manager::AssetManager::_AcquireInstance(std::string path, IO::Asset::IAsset* newAsset)
{
	_warps[path].refCount++;
	_warps[path].assets.emplace(newAsset);
	return _warps[path].assetInstance;
}

bool IO::Manager::AssetManager::_ContainsInstance(std::string path)
{
	return _warps.count(path);
}

void IO::Manager::AssetManager::_ReleaseInstance(IO::Asset::IAssetInstance* assetInstance, IO::Asset::IAsset* newAsset)
{
	if (_warps[assetInstance->path].assets.count(newAsset))
	{
		--_warps[assetInstance->path].refCount;
		_warps[assetInstance->path].assets.erase(newAsset);
	}
}
