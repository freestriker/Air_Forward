#pragma once
#include <string>
#include <mutex>
#include <map>
#include <future>
#include <iostream>
#include "IO/Core/Thread.h"
#include "Utils/Log.h"
#include <set>
#include "IO/Core/Instance.h"
#include "IO/Core/Thread.h"
#include "Utils/Log.h"
namespace Graphic
{
	namespace Command
	{
		class CommandBuffer;
	}
}
namespace IO
{
	namespace Manager
	{
		class AssetManager;
	}
	namespace Asset
	{
		class IAssetInstance;
		class IAsset
		{
			friend class Manager::AssetManager;
		private:
			IAsset& operator=(const IAsset&) = delete;
			IAsset(const IAsset&) = delete;
			IAsset(IAsset&&) = delete;
			IAsset& operator=(IAsset&&) = delete;
		protected:
			IAssetInstance* _assetInstance;
			IAsset();
			virtual ~IAsset();

			template<typename TAsset, typename TAssetInstance>
			inline static std::future<TAsset*> _LoadAsync(const char* path);

			template<typename TAsset, typename TAssetInstance>
			inline static TAsset* _Load(const char* path);

			template<typename TAsset, typename TAssetInstance>
			inline static void _Unload(TAsset* asset);
		};

		class IAssetInstance
		{
			friend class IAsset;
			friend class Manager::AssetManager;
		private:
			inline virtual void _LoadAssetInstance(Graphic::Command::CommandBuffer* const transferCommandBuffer) = 0;
			inline void _Wait();
			bool _readyToUse;
		protected:
			std::string const path;
			IAssetInstance(std::string path);
			virtual ~IAssetInstance();
		};

	}
}

inline void IO::Asset::IAssetInstance::_Wait()
{
	while (!this->_readyToUse)
	{
		std::this_thread::yield();
	}
}

template<typename TAsset, typename TAssetInstance>
inline std::future<TAsset*> IO::Asset::IAsset::_LoadAsync(const char* path)
{
	TAssetInstance* assetInstance = nullptr;
	TAsset* newAsset = new TAsset();
	bool alreadyCreated = false;

	{
		auto& manager = IO::Core::Instance::assetManager;

		std::unique_lock<std::mutex> lock(manager._managerMutex);

		if (manager._ContainsInstance(path))
		{
			assetInstance = dynamic_cast<TAssetInstance*>(manager._AcquireInstance(path, dynamic_cast<IAsset*>(newAsset)));
			alreadyCreated = true;
		}
		else
		{
			assetInstance = new TAssetInstance(path);
			manager._AddInstance(path, dynamic_cast<IAssetInstance*>(assetInstance));
			manager._AcquireInstance(path, dynamic_cast<IAsset*>(newAsset));
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
			Utils::Log::Message("AssetManager load " + sPath + " from asset pool.");
			return newAsset;
		});
	}
	else
	{
		return IO::Core::Thread::AddTask([assetInstance, sPath, newAsset](Graphic::Command::CommandBuffer* const tcb)
		{
			dynamic_cast<IAssetInstance*>(assetInstance)->_LoadAssetInstance(tcb);
			dynamic_cast<IAssetInstance*>(assetInstance)->_readyToUse = true;
			dynamic_cast<IAssetInstance*>(assetInstance)->_Wait();
			Utils::Log::Message("AssetManager load " + sPath + " from disk.");
			return newAsset;
		});
	}
}

template<typename TAsset, typename TAssetInstance>
inline TAsset* IO::Asset::IAsset::_Load(const char* path)
{
	return _LoadAsync<TAsset, TAssetInstance>(path).get();
}

template<typename TAsset, typename TAssetInstance>
inline void IO::Asset::IAsset::_Unload(TAsset* asset)
{
	auto& manager = IO::Core::Instance::assetManager;
	
	std::unique_lock<std::mutex> lock(manager._managerMutex);
	std::string unloaded = asset->_assetInstance->path;
	manager._ReleaseInstance(asset->_assetInstance, asset);
	delete asset;
	Utils::Log::Message("AssetManager unload " + unloaded + " .");
}
