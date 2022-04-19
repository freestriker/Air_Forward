#pragma once
#include <string>
#include <mutex>
#include <map>
class AssetManager;
class IAssetInstance;
class IAsset
{
protected:
	IAssetInstance* assetInstance;
	IAsset(IAssetInstance* assetInstance);
	virtual ~IAsset();
private:
	IAsset(const IAsset& source) = delete;
	IAsset& operator=(const IAsset&) = delete;
	IAsset(IAsset&&) = delete;
	IAsset& operator=(IAsset&&) = delete;
};
class IAssetInstance
{
public:
	std::string path;
	AssetManager* const assetManager;
	IAssetInstance(std::string path, AssetManager* assetManager);
	template<typename T, typename ...Args>
	static bool GetAssetInstance(AssetManager* manager, std::string path, T*& dstAssetInstance, Args && ...args);
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

template<typename T, typename ...Args>
inline bool IAssetInstance::GetAssetInstance(AssetManager* manager, std::string path, T*& dstAssetInstance, Args && ...args)
{
	std::unique_lock<std::mutex> lock(manager->mutex);
	T* asset = nullptr;
	bool alreadyCreated = false;
	if (manager->ContainsInstance(path))
	{
		asset = dynamic_cast<T*>(manager->GetInstance(path));
		alreadyCreated = true;
	}
	else
	{
		asset = new T(path, args...);
		manager->AddInstance(path, asset);
		manager->GetInstance(path);
		alreadyCreated = false;
	}

	dstAssetInstance = asset;
	return alreadyCreated;
}
