#include "Graphic/DescriptorSetUtils.h"
#include "Graphic/GlobalInstance.h"
#include <stdexcept>
#include <spirv_cross/spirv_reflect.hpp>


//
//Graphic::DescriptorSetLayout::DescriptorSetLayout(std::vector<DescriptorSetLayoutBinding> bindings)
//	: vkDescriptorSetLayout(_CreateDescriptorSetLayout(bindings))
//	, descriptorTypes(_GetDescriptorTypes(bindings))
//{
//}
//
//Graphic::DescriptorSetLayout::~DescriptorSetLayout()
//{
//	vkDestroyDescriptorSetLayout(Graphic::GlobalInstance::device, vkDescriptorSetLayout, nullptr);
//}
//
//std::vector<VkDescriptorType> Graphic::DescriptorSetLayout::_GetDescriptorTypes(std::vector<DescriptorSetLayoutBinding>& bindings)
//{
//	std::vector<VkDescriptorType> result = std::vector<VkDescriptorType>(bindings.size());
//	for (size_t i = 0; i < bindings.size(); i++)
//	{
//		result[i] = bindings[i].descriptorType;
//	}
//	return result;
//}
//
//VkDescriptorSetLayout Graphic::DescriptorSetLayout::_CreateDescriptorSetLayout(std::vector<DescriptorSetLayoutBinding>& bindings)
//{
//	VkDescriptorSetLayout result = VkDescriptorSetLayout();
//
//	std::vector<VkDescriptorSetLayoutBinding> vkBindings = std::vector<VkDescriptorSetLayoutBinding>(bindings.size());
//	for (size_t i = 0; i < bindings.size(); i++)
//	{
//		vkBindings[i].binding = static_cast<uint32_t>(i);
//		vkBindings[i].descriptorCount = 1;
//		vkBindings[i].descriptorType = bindings[i].descriptorType;
//		vkBindings[i].pImmutableSamplers = nullptr;
//		vkBindings[i].stageFlags = bindings[i].stageFlags;
//	}
//
//	VkDescriptorSetLayoutCreateInfo layoutInfo{};
//	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
//	layoutInfo.bindingCount = static_cast<uint32_t>(vkBindings.size());
//	layoutInfo.pBindings = vkBindings.data();
//
//	if (vkCreateDescriptorSetLayout(Graphic::GlobalInstance::device, &layoutInfo, nullptr, &result) != VK_SUCCESS)
//	{
//		throw std::runtime_error("failed to create descriptor set layout!");
//	}
//
//	return result;
//}
//
//Graphic::DescriptorSetLayoutBinding::DescriptorSetLayoutBinding(VkDescriptorType descriptorType, VkShaderStageFlags stageFlags)
//	: descriptorType(descriptorType)
//	, stageFlags(stageFlags)
//{
//}
//
//Graphic::DescriptorSetLayoutBinding::~DescriptorSetLayoutBinding()
//{
//}
//
//Graphic::DescriptorPool::DescriptorPool(DescriptorSetLayout* templateLayout, int chunkSize)
//	: _chunkSize(chunkSize)
//	, _templateLayout(templateLayout)
//	, _poolSizes(_GetPoolSizes(templateLayout, chunkSize))
//	, _pools()
//	, _poolRemainingCounts()
//{
//}
//
//Graphic::DescriptorPool::~DescriptorPool()
//{
//	for (const auto& pool : _pools)
//	{
//		vkResetDescriptorPool(Graphic::GlobalInstance::device, pool, 0);
//		vkDestroyDescriptorPool(Graphic::GlobalInstance::device, pool, nullptr);
//	}
//}
//
//Graphic::DescriptorSet* Graphic::DescriptorPool::GetDescripterSet()
//{
//	
//	VkDescriptorPool useablePool = VK_NULL_HANDLE;
//	for (size_t i = 0; i < _pools.size(); i++)
//	{
//		if (_poolRemainingCounts[_pools[i]] > 0)
//		{
//			useablePool = _pools[i];
//			break;
//		}
//	}
//
//	if (useablePool == VK_NULL_HANDLE)
//	{
//		auto i = _CreateNewPool();
//		useablePool = _pools[i];
//	}
//	--_poolRemainingCounts[useablePool];
//
//	VkDescriptorSetAllocateInfo allocInfo{};
//	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
//	allocInfo.descriptorPool = useablePool;
//	allocInfo.descriptorSetCount = 1;
//	allocInfo.pSetLayouts = &_templateLayout->vkDescriptorSetLayout;
//
//	VkDescriptorSet newVkSet = VK_NULL_HANDLE;
//	if (vkAllocateDescriptorSets(Graphic::GlobalInstance::device, &allocInfo, &newVkSet) != VK_SUCCESS)
//	{
//		throw std::runtime_error("failed to allocate descriptor sets!");
//	}
//
//	DescriptorSet* newSet = new DescriptorSet(useablePool, newVkSet);
//	return newSet;
//}
//
//void Graphic::DescriptorPool::RecycleDescripterSet(DescriptorSet* descriptorSet)
//{
//	for (size_t i = 0; i < _pools.size(); i++)
//	{
//		if (descriptorSet->sourcePool == _pools[i])
//		{
//			vkFreeDescriptorSets(Graphic::GlobalInstance::device, descriptorSet->sourcePool, 1, &descriptorSet->descriptorSet);
//			auto remainingCount = ++_poolRemainingCounts[_pools[i]];
//
//			if (remainingCount == _chunkSize)
//			{
//				_DestoryPool(i);
//			}
//			return;
//		}
//	}
//	throw std::runtime_error("Can not find the right pool.");
//}
//
//size_t Graphic::DescriptorPool::_CreateNewPool()
//{
//	VkDescriptorPoolCreateInfo poolInfo{};
//	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
//	poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
//	poolInfo.poolSizeCount = static_cast<uint32_t>(_poolSizes.size());
//	poolInfo.pPoolSizes = _poolSizes.data();
//	poolInfo.maxSets = _chunkSize;
//
//	VkDescriptorPool newPool = VK_NULL_HANDLE;
//	if (vkCreateDescriptorPool(Graphic::GlobalInstance::device, &poolInfo, nullptr, &newPool) != VK_SUCCESS) {
//		throw std::runtime_error("failed to create descriptor pool!");
//	}
//
//	_pools.push_back(newPool);
//	_poolRemainingCounts[newPool] = _chunkSize;
//
//	return _pools.size() - 1;
//}
//
//void Graphic::DescriptorPool::_DestoryPool(size_t index)
//{
//	_poolRemainingCounts.erase(_pools[index]);
//
//	vkResetDescriptorPool(Graphic::GlobalInstance::device, _pools[index], 0);
//	vkDestroyDescriptorPool(Graphic::GlobalInstance::device, _pools[index], nullptr);
//	_pools.erase(_pools.begin() + index);
//
//}
//
//std::vector<VkDescriptorPoolSize> Graphic::DescriptorPool::_GetPoolSizes(DescriptorSetLayout* templateLayout, int chunkSize)
//{
//	std::map<VkDescriptorType, uint32_t> typeCounts = std::map<VkDescriptorType, uint32_t>();
//	for (const auto& descriptorType : templateLayout->descriptorTypes)
//	{
//		if (!typeCounts.count(descriptorType))
//		{
//			typeCounts[descriptorType] = 0;
//		}
//		++typeCounts[descriptorType];
//	}
//
//	std::vector<VkDescriptorPoolSize> poolSizes = std::vector<VkDescriptorPoolSize>(typeCounts.size());
//	size_t poolSizeIndex = 0;
//	for (const auto& pair : typeCounts)
//	{
//		poolSizes[poolSizeIndex].type = pair.first;
//		poolSizes[poolSizeIndex].descriptorCount = pair.second * chunkSize;
//
//		poolSizeIndex++;
//	}
//
//	return poolSizes;
//}
//
//void Graphic::DescriptorSet::WriteBindingData(std::vector<Graphic::DescriptorSet::WriteData> data)
//{
//	std::vector< VkDescriptorBufferInfo> bufferInfos = std::vector< VkDescriptorBufferInfo>(data.size());
//	std::vector< VkDescriptorImageInfo> imageInfos = std::vector< VkDescriptorImageInfo>(data.size());
//	std::vector< VkWriteDescriptorSet> writeInfos = std::vector< VkWriteDescriptorSet>(data.size());
//
//	for (size_t i = 0; i < data.size(); i++)
//	{
//		if (data[i].type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
//		{
//			bufferInfos[i].buffer = data[i].buffer;
//			bufferInfos[i].offset = data[i].offset;
//			bufferInfos[i].range = data[i].range;
//
//			writeInfos[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
//			writeInfos[i].dstSet = descriptorSet;
//			writeInfos[i].dstBinding = static_cast<uint32_t>(i);
//			writeInfos[i].dstArrayElement = 0;
//			writeInfos[i].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
//			writeInfos[i].descriptorCount = 1;
//			writeInfos[i].pBufferInfo = &bufferInfos[i];
//		}
//		else if (data[i].type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
//		{
//			imageInfos[i].imageLayout = data[i].layout;
//			imageInfos[i].imageView = data[i].view;
//			imageInfos[i].sampler = data[i].sampler;
//
//			writeInfos[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
//			writeInfos[i].dstSet = descriptorSet;
//			writeInfos[i].dstBinding = static_cast<uint32_t>(i);
//			writeInfos[i].dstArrayElement = 0;
//			writeInfos[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
//			writeInfos[i].descriptorCount = 1;
//			writeInfos[i].pImageInfo = &imageInfos[i];
//		}
//	}
//	vkUpdateDescriptorSets(Graphic::GlobalInstance::device, static_cast<uint32_t>(writeInfos.size()), writeInfos.data(), 0, nullptr);
//
//}
//
//void Graphic::DescriptorSet::WriteBindingData(std::vector<uint32_t> bindingIndex, std::vector<WriteData> data)
//{
//	std::vector< VkDescriptorBufferInfo> bufferInfos = std::vector< VkDescriptorBufferInfo>(data.size());
//	std::vector< VkDescriptorImageInfo> imageInfos = std::vector< VkDescriptorImageInfo>(data.size());
//	std::vector< VkWriteDescriptorSet> writeInfos = std::vector< VkWriteDescriptorSet>(data.size());
//
//	for (size_t i = 0; i < data.size(); i++)
//	{
//		if (data[i].type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
//		{
//			bufferInfos[i].buffer = data[i].buffer;
//			bufferInfos[i].offset = data[i].offset;
//			bufferInfos[i].range = data[i].range;
//
//			writeInfos[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
//			writeInfos[i].dstSet = descriptorSet;
//			writeInfos[i].dstBinding = bindingIndex[i];
//			writeInfos[i].dstArrayElement = 0;
//			writeInfos[i].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
//			writeInfos[i].descriptorCount = 1;
//			writeInfos[i].pBufferInfo = &bufferInfos[i];
//		}
//		else if (data[i].type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
//		{
//			imageInfos[i].imageLayout = data[i].layout;
//			imageInfos[i].imageView = data[i].view;
//			imageInfos[i].sampler = data[i].sampler;
//
//			writeInfos[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
//			writeInfos[i].dstSet = descriptorSet;
//			writeInfos[i].dstBinding = bindingIndex[i];
//			writeInfos[i].dstArrayElement = 0;
//			writeInfos[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
//			writeInfos[i].descriptorCount = 1;
//			writeInfos[i].pImageInfo = &imageInfos[i];
//		}
//	}
//	vkUpdateDescriptorSets(Graphic::GlobalInstance::device, static_cast<uint32_t>(writeInfos.size()), writeInfos.data(), 0, nullptr);
//
//}
//
//Graphic::DescriptorSet::DescriptorSet(VkDescriptorPool sourcePool, VkDescriptorSet set)
//	: sourcePool(sourcePool)
//	, descriptorSet(set)
//{
//}
//Graphic::DescriptorSet::~DescriptorSet()
//{
//}
//
//Graphic::DescriptorSet::WriteData::WriteData(VkDescriptorType type, VkBuffer buffer, VkDeviceSize offset, VkDeviceSize range)
//	: type(type)
//	, buffer(buffer)
//	, offset(offset)
//	, range(range)
//{
//}
//
//Graphic::DescriptorSet::WriteData::WriteData(VkDescriptorType type, VkImageLayout layout, VkImageView view, VkSampler sampler)
//	: type(type)
//	, layout(layout)
//	, view(view)
//	, sampler(sampler)
//{
//}

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

Graphic::Manager::DescriptorSetManager::_DescriptorPool::_DescriptorPool(Asset::SlotType slotType, std::vector<VkDescriptorType>& types, int chunkSize)
	: slotType(slotType)
	, mutex()
	, chunkSize(chunkSize)
	, chunkSizes(GetPoolSizes(types, chunkSize))
	, chunks()
	, poolRemainingCounts()
{
}

Graphic::Manager::DescriptorSetManager::_DescriptorPool::~_DescriptorPool()
{
	for (const auto& chunk : chunks)
	{
		vkResetDescriptorPool(Graphic::GlobalInstance::device, chunk, 0);
		vkDestroyDescriptorPool(Graphic::GlobalInstance::device, chunk, nullptr);
	}
}

Graphic::Manager::DescriptorSet* Graphic::Manager::DescriptorSetManager::_DescriptorPool::AcquireDescripterSet(VkDescriptorSetLayout descriptorSetLayout)
{
	std::unique_lock<std::mutex> lock(mutex);

	VkDescriptorPool useableChunk = VK_NULL_HANDLE;
	for (size_t i = 0; i < chunks.size(); i++)
	{
		if (poolRemainingCounts[chunks[i]] > 0)
		{
			useableChunk = chunks[i];
			break;
		}
	}

	if (useableChunk == VK_NULL_HANDLE)
	{
		auto i = CreateNewPool();
		useableChunk = chunks[i];
	}
	--poolRemainingCounts[useableChunk];

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

void Graphic::Manager::DescriptorSetManager::_DescriptorPool::ReleaseDescripterSet(DescriptorSet* descriptorSet)
{
	for (size_t i = 0; i < chunks.size(); i++)
	{
		if (descriptorSet->_sourceDescriptorChunk == chunks[i])
		{
			vkFreeDescriptorSets(Graphic::GlobalInstance::device, descriptorSet->_sourceDescriptorChunk, 1, &descriptorSet->_descriptorSet);
			auto remainingCount = ++poolRemainingCounts[chunks[i]];

			if (remainingCount == chunkSize)
			{
				DestoryPool(i);
			}
			return;
		}

}

size_t Graphic::Manager::DescriptorSetManager::_DescriptorPool::CreateNewPool()
{
	return size_t();
}

void Graphic::Manager::DescriptorSetManager::_DescriptorPool::DestoryPool(size_t index)
{
}

std::vector<VkDescriptorPoolSize> Graphic::Manager::DescriptorSetManager::_DescriptorPool::GetPoolSizes(std::vector<VkDescriptorType>& types, int chunkSize)
{
	return std::vector<VkDescriptorPoolSize>();
}
