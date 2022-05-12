#pragma once
#include <string>
#include <mutex>
#include <map>
#include <future>
#include <iostream>
#include "LoadThread.h"
#include "utils/Log.h"
#include <set>
namespace Graphic
{
	class CommandBuffer;
}
class AssetManager;
class IAssetInstance;
class IAsset
{
	friend class AssetManager;
protected:
	IAssetInstance* _assetInstance;
	IAsset();
	virtual ~IAsset();

	template<typename TAsset, typename TAssetInstance>
	static std::future<TAsset*> _LoadAsync(const char* path);

	template<typename TAsset, typename TAssetInstance>
	static TAsset* _Load(const char* path);

	template<typename TAsset, typename TAssetInstance>
	static void _Unload(TAsset* asset);
private:
	IAsset& operator=(const IAsset&) = delete;
	IAsset(const IAsset&) = delete;
	IAsset(IAsset&&) = delete;
	IAsset& operator=(IAsset&&) = delete;
};
class IAssetInstance
{
	friend class IAsset;
	friend class AssetManager;
private:
	virtual void _LoadAssetInstance(Graphic::CommandBuffer* const transferCommandBuffer, Graphic::CommandBuffer* const renderCommandBuffer) = 0;
	void _Wait();
	bool _readyToUse;
public:
	std::string const path;
	IAssetInstance(std::string path);
	virtual ~IAssetInstance();
};
class AssetManager
{
	friend class IAssetInstance;
	friend class IAsset;
private:
	class _AssetInstanceWarp
	{
	public:
		uint32_t refCount;
		std::set<IAsset*> assets;
		IAssetInstance* assetInstance;
	};
	std::mutex _mutex;
	std::map<std::string, _AssetInstanceWarp> _warps;

public:
	AssetManager();
	~AssetManager();
	void Collect();
	void _AddInstance(std::string path, IAssetInstance* assetInstance);
	IAssetInstance* _AcquireInstance(std::string path, IAsset* newAsset);
	bool _ContainsInstance(std::string path);
	void _ReleaseInstance(IAssetInstance* assetInstance, IAsset* newAsset);
};

template<typename TAsset, typename TAssetInstance>
inline std::future<TAsset*> IAsset::_LoadAsync(const char* path)
{
	TAssetInstance* assetInstance = nullptr;
	TAsset* newAsset = new TAsset();
	bool alreadyCreated = false;

	{
		auto manager = LoadThread::instance->assetManager.get();

		std::unique_lock<std::mutex> lock(manager->_mutex);

		if (manager->_ContainsInstance(path))
		{
			assetInstance = dynamic_cast<TAssetInstance*>(manager->_AcquireInstance(path, dynamic_cast<IAsset*>(newAsset)));
			alreadyCreated = true;
		}
		else
		{
			assetInstance = new TAssetInstance(path);
			manager->_AddInstance(path, dynamic_cast<IAssetInstance*>(assetInstance));
			manager->_AcquireInstance(path, dynamic_cast<IAsset*>(newAsset));
			alreadyCreated = false;
		}
	}
	dynamic_cast<IAsset*>(newAsset)->_assetInstance = dynamic_cast<IAssetInstance*>(assetInstance);
	std::string sPath = std::string(path);
	if (alreadyCreated)
	{
		return std::async([assetInstance, sPath, newAsset]()
		{
			dynamic_cast<IAssetInstance*>(assetInstance)->_Wait();
			Log::Message("AssetManager load " + sPath + " from asset pool.");
			return newAsset;
		});
	}
	else
	{
		return LoadThread::instance->AddTask([assetInstance, sPath, newAsset](Graphic::CommandBuffer* const tcb, Graphic::CommandBuffer* const gcb)
		{
			dynamic_cast<IAssetInstance*>(assetInstance)->_LoadAssetInstance(tcb, gcb);
			dynamic_cast<IAssetInstance*>(assetInstance)->_readyToUse = true;
			dynamic_cast<IAssetInstance*>(assetInstance)->_Wait();
			Log::Message("AssetManager load " + sPath + " from disk.");
			return newAsset;
		});
	}
}

template<typename TAsset, typename TAssetInstance>
inline TAsset* IAsset::_Load(const char* path)
{
	return _LoadAsync<TAsset, TAssetInstance>(path).get();
}

template<typename TAsset, typename TAssetInstance>
inline void IAsset::_Unload(TAsset* asset)
{
	std::unique_lock<std::mutex> lock(LoadThread::instance->assetManager->_mutex);
	std::string unloaded = asset->_assetInstance->path;
	LoadThread::instance->assetManager->_ReleaseInstance(asset->_assetInstance, asset);
	delete asset;
	Log::Message("AssetManager unload " + unloaded + " .");
}
