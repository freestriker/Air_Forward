#include "Graphic/Manager/DescriptorSetManager.h"
#include "Graphic/GlobalInstance.h"
#include <stdexcept>
#include <spirv_cross/spirv_reflect.hpp>
#include "Graphic/Instance/DescriptorSet.h"
#include "utils/Log.h"

Graphic::Manager::DescriptorSetManager::_DescriptorPool::_DescriptorPool(Asset::SlotType slotType, std::vector<VkDescriptorType>& types, uint32_t chunkSize)
	: slotType(slotType)
	, mutex()
	, chunkSize(chunkSize)
	, poolSizes(GetPoolSizes(types, chunkSize))
	, chunkCreateInfo(GetChunkCreateInfo(chunkSize, poolSizes))
	, chunks()
	, handles()
{
}

Graphic::Manager::DescriptorSetManager::_DescriptorPool::~_DescriptorPool()
{
	std::unique_lock<std::mutex> lock(mutex);

	for (const auto& handle : handles)
	{
		vkFreeDescriptorSets(Graphic::GlobalInstance::device, handle->_sourceVkDescriptorChunk, 1, &handle->_vkDescriptorSet);

		delete handle;
	}

	for (const auto& chunkPair : chunks)
	{
		vkResetDescriptorPool(Graphic::GlobalInstance::device, chunkPair.first, 0);
		vkDestroyDescriptorPool(Graphic::GlobalInstance::device, chunkPair.first, nullptr);
	}
}

Graphic::Instance::DescriptorSet* Graphic::Manager::DescriptorSetManager::_DescriptorPool::AcquireDescripterSet(VkDescriptorSetLayout descriptorSetLayout)
{
	std::unique_lock<std::mutex> lock(mutex);

	VkDescriptorPool useableChunk = VK_NULL_HANDLE;
	for (const auto& chunkPair : chunks)
	{
		if (chunkPair.second > 0)
		{
			useableChunk = chunkPair.first;
			break;
		}
	}
	if (useableChunk == VK_NULL_HANDLE)
	{
		Log::Exception("Failed to create descriptor chunk.", vkCreateDescriptorPool(Graphic::GlobalInstance::device, &chunkCreateInfo, nullptr, &useableChunk));
		chunks.emplace(useableChunk, chunkSize);
	}
	--chunks[useableChunk];

	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = useableChunk;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = &descriptorSetLayout;

	VkDescriptorSet newVkSet = VK_NULL_HANDLE;
	Log::Exception("Failed to allocate descriptor sets.", vkAllocateDescriptorSets(Graphic::GlobalInstance::device, &allocInfo, &newVkSet));

	Instance::DescriptorSet* newSet = new Instance::DescriptorSet(slotType, newVkSet, useableChunk);
	
	handles.emplace(newSet);

	return newSet;
}

void Graphic::Manager::DescriptorSetManager::_DescriptorPool::ReleaseDescripterSet(Instance::DescriptorSetHandle descriptorSet)
{
	std::unique_lock<std::mutex> lock(mutex);

	++chunks[descriptorSet->_sourceVkDescriptorChunk];
	vkFreeDescriptorSets(Graphic::GlobalInstance::device, descriptorSet->_sourceVkDescriptorChunk, 1, &descriptorSet->_vkDescriptorSet);

	handles.erase(descriptorSet);

	delete descriptorSet;
}

void Graphic::Manager::DescriptorSetManager::_DescriptorPool::CollectEmptyChunk()
{
	std::unique_lock<std::mutex> lock(mutex);

	for (auto it = chunks.cbegin(); it != chunks.cend(); )
	{
		if (it->second == 0)
		{
			vkResetDescriptorPool(Graphic::GlobalInstance::device, it->first, 0);
			vkDestroyDescriptorPool(Graphic::GlobalInstance::device, it->first, nullptr);
			it = chunks.erase(it);
		}
		else
		{
			++it;
		}
	}
}

std::vector<VkDescriptorPoolSize> Graphic::Manager::DescriptorSetManager::_DescriptorPool::GetPoolSizes(std::vector<VkDescriptorType>& types, int chunkSize)
{
	std::map<VkDescriptorType, uint32_t> typeCounts = std::map<VkDescriptorType, uint32_t>();
	for (const auto& descriptorType : types)
	{
		if (!typeCounts.count(descriptorType))
		{
			typeCounts[descriptorType] = 0;
		}
		++typeCounts[descriptorType];
	}
	
	std::vector<VkDescriptorPoolSize> poolSizes = std::vector<VkDescriptorPoolSize>(typeCounts.size());
	size_t poolSizeIndex = 0;
	for (const auto& pair : typeCounts)
	{
		poolSizes[poolSizeIndex].type = pair.first;
		poolSizes[poolSizeIndex].descriptorCount = pair.second * chunkSize;
	
		poolSizeIndex++;
	}
	
	return poolSizes;
}

VkDescriptorPoolCreateInfo Graphic::Manager::DescriptorSetManager::_DescriptorPool::GetChunkCreateInfo(uint32_t chunkSize, std::vector<VkDescriptorPoolSize> const& chunkSizes)
{
	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	poolInfo.poolSizeCount = static_cast<uint32_t>(chunkSizes.size());
	poolInfo.pPoolSizes = chunkSizes.data();
	poolInfo.maxSets = chunkSize;
	return poolInfo;
}

void Graphic::Manager::DescriptorSetManager::AddDescriptorSetPool(Asset::SlotType slotType, std::vector<VkDescriptorType> descriptorTypes, uint32_t chunkSize)
{
	std::unique_lock<std::shared_mutex> lock(_managerMutex);

	_pools[slotType] = new _DescriptorPool(slotType, descriptorTypes, chunkSize);
}

void Graphic::Manager::DescriptorSetManager::DeleteDescriptorSetPool(Asset::SlotType slotType)
{
	std::unique_lock<std::shared_mutex> lock(_managerMutex);

	delete _pools[slotType];
	_pools.erase(slotType);
}

Graphic::Instance::DescriptorSet* Graphic::Manager::DescriptorSetManager::AcquireDescripterSet(Asset::SlotType slotType, VkDescriptorSetLayout descriptorSetLayout)
{
	std::shared_lock<std::shared_mutex> lock(_managerMutex);

	return _pools[slotType]->AcquireDescripterSet(descriptorSetLayout);
}

void Graphic::Manager::DescriptorSetManager::ReleaseDescripterSet(Instance::DescriptorSetHandle descriptorSet)
{
	std::shared_lock<std::shared_mutex> lock(_managerMutex);

	_pools[descriptorSet->_slotType]->ReleaseDescripterSet(descriptorSet);
}

void Graphic::Manager::DescriptorSetManager::Collect()
{
	std::unique_lock<std::shared_mutex> lock(_managerMutex);

	for (auto& poolPair : _pools)
	{
		poolPair.second->CollectEmptyChunk();
	}
}

Graphic::Manager::DescriptorSetManager::DescriptorSetManager()
	: _managerMutex()
	, _pools()
{
}

Graphic::Manager::DescriptorSetManager::~DescriptorSetManager()
{
	std::unique_lock<std::shared_mutex> lock(_managerMutex);
	for (auto& poolPair : _pools)
	{
		delete poolPair.second;
	}
}
