#pragma once
#include <map>
#include <string>
#include <vulkan/vulkan_core.h>

namespace Graphic
{
	namespace Manager
	{
		class DescriptorSet;
		typedef DescriptorSet* DescriptorSetHandle;
	}
	class CommandBuffer;
	class Texture2D;
	namespace Asset
	{
		enum class SlotType;
		class Shader;
		class UniformBuffer;
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
		};

	private:
		Asset::Shader* _shader;
		std::map<std::string, _Slot> _slots;
	public:
		Material(Asset::Shader* shader);
		const Texture2D* GetTexture2D(const char* name);
		void SetTexture2D(const char* name, Texture2D* texture2d);
		const Asset::UniformBuffer* GetUniformBuffer(const char* name);
		void SetUniformBuffer(const char* name, Asset::UniformBuffer* texture2d);
		~Material();
	};
}