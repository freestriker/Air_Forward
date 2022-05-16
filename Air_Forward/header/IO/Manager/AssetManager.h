#pragma once
#include <cstdint>
#include <set>
#include <map>
#include <mutex>

namespace IO
{
	namespace Asset
	{
		class IAssetInstance;
		class IAsset;
	}
	namespace Manager
	{
		class AssetManager final
		{
			friend class Asset::IAssetInstance;
			friend class Asset::IAsset;
		private:
			class _AssetInstanceWarp
			{
			public:
				uint32_t refCount;
				std::set<IO::Asset::IAsset*> assets;
				IO::Asset::IAssetInstance* assetInstance;
			};
			std::mutex _managerMutex;
			std::map<std::string, _AssetInstanceWarp> _warps;

			void _AddInstance(std::string path, Asset::IAssetInstance* assetInstance);
			Asset::IAssetInstance* _AcquireInstance(std::string path, Asset::IAsset* newAsset);
			bool _ContainsInstance(std::string path);
			void _ReleaseInstance(Asset::IAssetInstance* assetInstance, Asset::IAsset* newAsset);
		public:
			AssetManager();
			~AssetManager();
			void Collect();
		};
	}
}
