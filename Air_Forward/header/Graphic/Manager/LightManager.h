#pragma once
#include <array>
#include <glm/glm.hpp>
#include <vector>

namespace Logic
{
	namespace Component
	{
		class Component;
		namespace Light
		{
			class Light;
		}
	}
}
namespace Graphic
{
	namespace Core
	{
		class Instance;
		class Thread;
	}
	namespace Instance
	{
		class Buffer;
	}
	namespace Command
	{
		class CommandPool;
		class CommandBuffer;
	}
	namespace Manager
	{
		class LightManager final
		{
		public:
			struct LightData
			{
				alignas(4) int type;
				alignas(4) float intensity;
				alignas(4) float range;
				alignas(4) float extraParamter;
				alignas(16) glm::vec3 position;
				alignas(16) glm::vec4 color;
			};
			void SetLightData(std::vector<Logic::Component::Component*> lights);
			void CopyLightData(Command::CommandBuffer* commandBuffer);
			LightManager();
			~LightManager();
		private:
			Instance::Buffer* _stageBuffer;
			Instance::Buffer* _mainLightBuffer;
			Instance::Buffer* _importantLightsBuffer;
			Instance::Buffer* _unimportantLightsBuffer;
			LightData _mainLightData;
			std::array<LightData, 4> _importantLightData;
			std::array<LightData, 4> _unimportantLightData;

			LightManager(const LightManager&) = delete;
			LightManager& operator=(const LightManager&) = delete;
			LightManager(LightManager&&) = delete;
			LightManager& operator=(LightManager&&) = delete;
		};
	}
}