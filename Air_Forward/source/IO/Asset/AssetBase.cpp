#include "IO/Asset/AssetBase.h"

IO::Asset::IAsset::IAsset()
	: _assetInstance(nullptr)
{
}

IO::Asset::IAsset::~IAsset()
{
}

IO::Asset::IAssetInstance::IAssetInstance(std::string path)
	: path(path)
	, _readyToUse(false)
{
}

IO::Asset::IAssetInstance::~IAssetInstance()
{

}