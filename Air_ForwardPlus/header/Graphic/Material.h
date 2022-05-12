#pragma once
#include <map>
#include <string>
#include <vulkan/vulkan_core.h>
#include <vector>

namespace Graphic
{
	namespace Manager
	{
		class DescriptorSet;
		typedef DescriptorSet* DescriptorSetHandle;
	}
	class CommandBuffer;
	namespace Asset
	{
		enum class SlotType;
		class Shader;
		class Texture2D;
	}
	namespace Instance
	{
		class Buffer;
	}


	class Material
	{
	private:
		struct _Slot
		{
			std::string name;
			void* asset;
			Asset::SlotType slotType;
			Manager::DescriptorSetHandle descriptorSet;
			uint32_t set;
		};

	private:
		Asset::Shader* _shader;
		std::map<std::string, _Slot> _slots;
	public:
		Material(Asset::Shader* shader);
		const Asset::Texture2D* GetTexture2D(const char* name);
		void SetTexture2D(const char* name, Asset::Texture2D* texture2d);
		const Instance::Buffer* GetUniformBuffer(const char* name);
		void SetUniformBuffer(const char* name, Instance::Buffer* texture2d);
		void RefreshSlotData(std::vector<std::string> slotNames);
		VkPipelineLayout PipelineLayout();
		std::vector<VkDescriptorSet> DescriptorSets();

		~Material();
	};
}