#pragma once
#include <string>
#include <mutex>
#include <map>
class AssetManager;
class IAssetInstance;
class IAsset
{
public:
	IAssetInstance* assetInstance;
	IAsset(IAssetInstance* assetInstance);
	IAsset(const IAsset& source) = delete;
	IAsset& operator=(const IAsset&) = delete;
	IAsset(IAsset&&) = delete;
	IAsset& operator=(IAsset&&) = delete;
	virtual ~IAsset();
};
class IAssetInstance
{
public:
	std::string path;
	AssetManager* assetManager;
	IAssetInstance(std::string path, AssetManager* assetManager);
	virtual ~IAssetInstance();
};
class AssetManager
{
public:
	class AssetInstanceWarp
	{
	public:
		std::string path;
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
