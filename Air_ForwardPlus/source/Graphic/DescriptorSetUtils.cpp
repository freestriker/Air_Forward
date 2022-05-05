#include "Graphic/DescriptorSetUtils.h"
#include "Graphic/GlobalInstance.h"
#include <stdexcept>
#include <spirv_cross/spirv_reflect.hpp>

VkDescriptorSet Graphic::Manager::DescriptorSet::Set()
{
	return _descriptorSet;
}

VkDescriptorSetLayout Graphic::Manager::DescriptorSet::SetLayout()
{
	return _descriptorSetLayout;
}

Graphic::Manager::DescriptorSet::DescriptorSet(Asset::SlotType slotType, VkDescriptorSet set, VkDescriptorSetLayout descriptorSetLayout, VkDescriptorPool sourceDescriptorChunk)
	: _slotType(slotType)
	, _descriptorSet(set)
	, _descriptorSetLayout(descriptorSetLayout)
	, _sourceDescriptorChunk(sourceDescriptorChunk)
{
}

Graphic::Manager::DescriptorSet::~DescriptorSet()
{
}

Graphic::Manager::DescriptorSetManager::_DescriptorPool::_DescriptorPool(Asset::SlotType slotType, std::vector<VkDescriptorType>& types, uint32_t chunkSize)
	: slotType(slotType)
	, mutex()
	, chunkSize(chunkSize)
	, chunkSizes(GetPoolSizes(types, chunkSize))
	, chunkCreateInfo(GetChunkCreateInfo(chunkSize, chunkSizes))
	, chunks()
{
}

Graphic::Manager::DescriptorSetManager::_DescriptorPool::~_DescriptorPool()
{
	std::unique_lock<std::mutex> lock(mutex);

	for (const auto& chunkPair : chunks)
	{
		vkResetDescriptorPool(Graphic::GlobalInstance::device, chunkPair.first, 0);
		vkDestroyDescriptorPool(Graphic::GlobalInstance::device, chunkPair.first, nullptr);
	}
}

Graphic::Manager::DescriptorSet* Graphic::Manager::DescriptorSetManager::_DescriptorPool::AcquireDescripterSet(VkDescriptorSetLayout descriptorSetLayout)
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
		VkDescriptorPool newChunk = VK_NULL_HANDLE;
		if (vkCreateDescriptorPool(Graphic::GlobalInstance::device, &chunkCreateInfo, nullptr, &newChunk) != VK_SUCCESS) {
			throw std::runtime_error("failed to create descriptor pool!");
		}
		chunks.emplace(newChunk, chunkSize);

		useableChunk = newChunk;
	}
	--chunks[useableChunk];

	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = useableChunk;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = &descriptorSetLayout;

	VkDescriptorSet newVkSet = VK_NULL_HANDLE;
	if (vkAllocateDescriptorSets(Graphic::GlobalInstance::device, &allocInfo, &newVkSet) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate descriptor sets!");
	}

	DescriptorSet* newSet = new DescriptorSet(slotType, newVkSet, descriptorSetLayout, useableChunk);
	return newSet;
}

void Graphic::Manager::DescriptorSetManager::_DescriptorPool::ReleaseDescripterSet(DescriptorSetHandle descriptorSet)
{
	std::unique_lock<std::mutex> lock(mutex);

	++chunks[descriptorSet->_sourceDescriptorChunk];
	vkFreeDescriptorSets(Graphic::GlobalInstance::device, descriptorSet->_sourceDescriptorChunk, 1, &descriptorSet->_descriptorSet);

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

Graphic::Manager::DescriptorSet* Graphic::Manager::DescriptorSetManager::AcquireDescripterSet(Asset::SlotType slotType, VkDescriptorSetLayout descriptorSetLayout)
{
	std::shared_lock<std::shared_mutex> lock(_managerMutex);

	return _pools[slotType]->AcquireDescripterSet(descriptorSetLayout);
}

void Graphic::Manager::DescriptorSetManager::ReleaseDescripterSet(DescriptorSetHandle descriptorSet)
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
void Graphic::Manager::DescriptorSet::WriteBindingData(std::vector<uint32_t> bindingIndex, std::vector< Graphic::Manager::DescriptorSet::DescriptorSetWriteData> data)
{
	std::vector< VkDescriptorBufferInfo> bufferInfos = std::vector< VkDescriptorBufferInfo>(data.size());
	std::vector< VkDescriptorImageInfo> imageInfos = std::vector< VkDescriptorImageInfo>(data.size());
	std::vector< VkWriteDescriptorSet> writeInfos = std::vector< VkWriteDescriptorSet>(data.size());

	for (size_t i = 0; i < data.size(); i++)
	{
		if (data[i].type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
		{
			bufferInfos[i].buffer = data[i].buffer;
			bufferInfos[i].offset = data[i].offset;
			bufferInfos[i].range = data[i].range;

			writeInfos[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			writeInfos[i].dstSet = _descriptorSet;
			writeInfos[i].dstBinding = bindingIndex[i];
			writeInfos[i].dstArrayElement = 0;
			writeInfos[i].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			writeInfos[i].descriptorCount = 1;
			writeInfos[i].pBufferInfo = &bufferInfos[i];
		}
		else if (data[i].type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
		{
			imageInfos[i].imageLayout = data[i].imageLayout;
			imageInfos[i].imageView = data[i].imageView;
			imageInfos[i].sampler = data[i].sampler;

			writeInfos[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			writeInfos[i].dstSet = _descriptorSet;
			writeInfos[i].dstBinding = bindingIndex[i];
			writeInfos[i].dstArrayElement = 0;
			writeInfos[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			writeInfos[i].descriptorCount = 1;
			writeInfos[i].pImageInfo = &imageInfos[i];
		}
	}
	vkUpdateDescriptorSets(Graphic::GlobalInstance::device, static_cast<uint32_t>(writeInfos.size()), writeInfos.data(), 0, nullptr);

}

Graphic::Manager::DescriptorSet::DescriptorSetWriteData::DescriptorSetWriteData(VkDescriptorType type, VkBuffer buffer, VkDeviceSize offset, VkDeviceSize range)
	: type(type)
	, buffer(buffer)
	, offset(offset)
	, range(range)
{
}

Graphic::Manager::DescriptorSet::DescriptorSetWriteData::DescriptorSetWriteData(VkDescriptorType type, VkSampler sampler, VkImageView imageView, VkImageLayout imageLayout)
	: type(type)
	, sampler(sampler)
	, imageView(imageView)
	, imageLayout(imageLayout)
{
}
