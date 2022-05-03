#pragma once
#include <map>
#include <string>

namespace Graphic
{
	class CommandBuffer;

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
		~Material();
	};
}