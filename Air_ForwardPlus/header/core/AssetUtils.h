#pragma once
#include <string>
#include <mutex>
#include <map>
#include <future>
#include <iostream>
#include "LoadThread.h"
namespace Graphic
{
	class CommandBuffer;
}
class AssetManager;
class IAssetInstance;
class IAsset
{
protected:
	IAssetInstance* _assetInstance;
	IAsset(IAssetInstance* assetInstance);
	IAsset(const IAsset& source);
	virtual ~IAsset();
	template<typename TAsset, typename TAssetInstance>
	static std::future<TAsset*> _LoadAsync(const char* path);
	template<typename TAsset, typename TAssetInstance>
	static TAsset* _Load(const char* path);
private:
	IAsset& operator=(const IAsset&) = delete;
	IAsset(IAsset&&) = delete;
	IAsset& operator=(IAsset&&) = delete;
};
class IAssetInstance
{
	friend class IAsset;
private:
	virtual void _LoadAssetInstance(Graphic::CommandBuffer* const transferCommandBuffer, Graphic::CommandBuffer* const renderCommandBuffer) = 0;
	void _Wait();
	bool _readyToUse;
public:
	std::string const path;
	AssetManager* const assetManager;
	IAssetInstance(std::string path);
	virtual ~IAssetInstance();
};
class AssetManager
{
public:
	class AssetInstanceWarp
	{
	public:
		uint32_t refCount;
		IAssetInstance* assetInstance;
	};
	std::mutex mutex;
private:
	std::map<std::string, AssetInstanceWarp> _warps;

public:
	AssetManager();
	~AssetManager();
	void AddInstance(std::string path, IAssetInstance* assetInstance);
	IAssetInstance* GetInstance(std::string path);
	bool ContainsInstance(std::string path);
	void RecycleInstance(std::string path);
};

template<typename TAsset, typename TAssetInstance>
inline std::future<TAsset*> IAsset::_LoadAsync(const char* path)
{
	TAssetInstance* assetInstance = nullptr;
	bool alreadyCreated = false;

	{
		auto manager = LoadThread::instance->assetManager.get();

		std::unique_lock<std::mutex> lock(manager->mutex);

		if (manager->ContainsInstance(path))
		{
			assetInstance = dynamic_cast<TAssetInstance*>(manager->GetInstance(path));
			alreadyCreated = true;
		}
		else
		{
			assetInstance = new TAssetInstance(path);
			manager->AddInstance(path, dynamic_cast<IAssetInstance*>(assetInstance));
			manager->GetInstance(path);
			alreadyCreated = false;
		}
	}
	std::string sPath = std::string(path);
	if (alreadyCreated)
	{
		return std::async([assetInstance, sPath]()
		{
			dynamic_cast<IAssetInstance*>(assetInstance)->_Wait();
			std::cerr << sPath << " load from asset pool." << std::endl;
			return new TAsset(assetInstance);
		});
	}
	else
	{
		return LoadThread::instance->AddTask([assetInstance, sPath](Graphic::CommandBuffer* const tcb, Graphic::CommandBuffer* const gcb)
		{
			dynamic_cast<IAssetInstance*>(assetInstance)->_LoadAssetInstance(tcb, gcb);
			dynamic_cast<IAssetInstance*>(assetInstance)->_readyToUse = true;
			dynamic_cast<IAssetInstance*>(assetInstance)->_Wait();
			std::cerr << sPath << " load from disk." << std::endl;
			return new TAsset(assetInstance);
		});
	}
}

template<typename TAsset, typename TAssetInstance>
inline TAsset* IAsset::_Load(const char* path)
{
	return _LoadAsync<TAsset, TAssetInstance>(path).get();
}
