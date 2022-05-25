#include "Graphic/Manager/LightManager.h"
#include "Graphic/Command/CommandBuffer.h"
#include "Logic/Component/Light/Light.h"
#include "Graphic/Instance/Buffer.h"
#include <map>
#include "Utils/Log.h"

void Graphic::Manager::LightManager::SetLightData(std::vector<Logic::Component::Component*> lights)
{
	std::multimap<Logic::Component::Light::Light::LightType, Logic::Component::Light::Light*> lightMap = std::multimap<Logic::Component::Light::Light::LightType, Logic::Component::Light::Light*>();
	for (const auto& lightComponent : lights)
	{
		Logic::Component::Light::Light* light = static_cast<Logic::Component::Light::Light*>(lightComponent);
		lightMap.emplace(std::make_pair( light->lightType, light ));
	}

	//main
	auto directionalIterator = lightMap.find(Logic::Component::Light::Light::LightType::DIRECTIONAL);
	if (directionalIterator != std::end(lightMap))
	{
		auto data = directionalIterator->second->GetLightData();
		_mainLightData = *reinterpret_cast<LightData*>(&data);
	}
	else
	{
		_mainLightData = {};
	}

	//important
	auto pointIterator = lightMap.find(Logic::Component::Light::Light::LightType::POINT);
	for (uint32_t i = 0; i < 4; i++)
	{
		if (pointIterator != std::end(lightMap))
		{
			auto data = pointIterator->second->GetLightData();
			_importantLightData[i] = *reinterpret_cast<LightData*>(&data);
			pointIterator++;
		}
		else
		{
			_importantLightData[i] = {};
		}
	}

	//unimportant
	for (uint32_t i = 0; i < 4; i++)
	{
		if (pointIterator != std::end(lightMap))
		{
			auto data = pointIterator->second->GetLightData();
			_unimportantLightData[i] = *reinterpret_cast<LightData*>(&data);
			pointIterator++;
		}
		else
		{
			_unimportantLightData[i] = {};
		}
	}
}

void Graphic::Manager::LightManager::CopyLightData(Command::CommandBuffer* commandBuffer)
{
	_stageBuffer->WriteBuffer([this](void* pointer) -> void{
		VkDeviceSize dataSize = sizeof(LightData);
		memcpy(pointer, &_mainLightData, dataSize);
		memcpy(reinterpret_cast<char*>(pointer) + dataSize, _importantLightData.data(), dataSize * 4);
		memcpy(reinterpret_cast<char*>(pointer) + dataSize * 5, _unimportantLightData.data(), dataSize * 4);
	});

	VkDeviceSize dataSize = sizeof(LightData);
	commandBuffer->Reset();
	commandBuffer->BeginRecord(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
	commandBuffer->CopyBuffer(_stageBuffer, 0, _mainLightBuffer, 0, dataSize);
	commandBuffer->CopyBuffer(_stageBuffer, dataSize, _importantLightsBuffer, 0, dataSize * 4);
	commandBuffer->CopyBuffer(_stageBuffer, dataSize * 5, _unimportantLightsBuffer, 0, dataSize * 4);
	commandBuffer->EndRecord();
	commandBuffer->Submit({}, {}, {});
	commandBuffer->WaitForFinish();
}

Graphic::Manager::LightManager::LightManager()
	: _stageBuffer(nullptr)
	, _mainLightBuffer(nullptr)
	, _importantLightsBuffer(nullptr)
	, _unimportantLightsBuffer(nullptr)
	, _mainLightData()
	, _importantLightData()
	, _unimportantLightData()
{
	VkDeviceSize dataSize = sizeof(LightData);
	_stageBuffer = new Instance::Buffer(dataSize * 9, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	_mainLightBuffer = new Instance::Buffer(dataSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	_importantLightsBuffer = new Instance::Buffer(dataSize * 4, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	_unimportantLightsBuffer = new Instance::Buffer(dataSize * 4, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
}

Graphic::Manager::LightManager::~LightManager()
{
}

Graphic::Instance::Buffer* Graphic::Manager::LightManager::MainLightBuffer()
{
	return _mainLightBuffer;
}

Graphic::Instance::Buffer* Graphic::Manager::LightManager::ImportantLightsBuffer()
{
	return _importantLightsBuffer;
}

Graphic::Instance::Buffer* Graphic::Manager::LightManager::UnimportantLightsBuffer()
{
	return _unimportantLightsBuffer;
}
