#pragma once
#include <map>
#include <string>
#include <vulkan/vulkan_core.h>

namespace Graphic
{
	class CommandBuffer;
	class Texture2D;
	namespace Asset
	{
		enum class SlotType;
		class Shader;
	}


	class Material
	{
	private:
		struct _Slot
		{
			std::string name;
			void* asset;
			VkDescriptorSet vkDescriptorSet;
			Asset::SlotType slotType;
		};

	private:
		Asset::Shader* _shader;
		std::map<std::string, _Slot> _slots;
	public:
		Material(Asset::Shader* shader);
		const Texture2D* GetTexture2D(const char* name);
		void SetTexture2D(const char* name, Texture2D* texture2d);
		~Material();
	};
}