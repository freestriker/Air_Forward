#pragma once
#include <map>
#include <string>

namespace Graphic
{
	class CommandBuffer;
	class Texture2D;
	namespace Asset
	{
		class Shader;
	}


	class Material
	{
	private:
		enum class _SlotType
		{
			UNIFORM_BUFFER,
			TEXTURE2D
		};
		struct _Slot
		{
			std::string name;
			void* asset;
			_SlotType slotType;
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